#include "protocol.h"
#include "tcp_server.h"

namespace protocol
{

std::shared_ptr<protocol> create_command_line_protocol(std::shared_ptr<tcp_server::tcp_server>& srv,
                                                       std::shared_ptr<app::file_manager_app>& app)
{
    auto prtcl = std::make_shared<command_line_protocol>(srv->get_logger());
    prtcl->set_application(app);

    prtcl->add_command(std::make_unique<exit_command>(srv, srv->get_logger()));
    prtcl->add_command(std::make_unique<help_command>(srv->get_logger()));
    prtcl->add_command(std::make_unique<get_files_command>(*app,srv->get_logger()));

    return std::move(prtcl);
}

//---------------------------------------------------------------
namespace
{

std::vector<std::string> split(std::string &in)
{
    std::vector<std::string> v;

    size_t first = 0, i = 0;
    size_t sz = in.size();
    while (i < sz)
    {
        if (std::isspace(in[i]))
        {
            if (first == i)
            {
                ++first;
            }
            else
            {
                v.push_back(in.substr(first, i - first));
                first = i + 1;
            }
        }
        ++i;
    }
    if (first != i)
        v.push_back(in.substr(first, i - first));
    return v;
}

} // namespace
//---------------------------------------------------------------

command::~command() {}
//---------------------------------------------------------------
help_command::~help_command() {}

eCodeError help_command::execute(std::weak_ptr<tcp_server::tcp_connection> connection,
                           std::vector<std::string>& command_line)
{
    if(command_line.size() == 0) return eCodeError::CommandLineIsEmpty;

    if(command_line[0] != get_key_word()) return eCodeError::KeyWordIsNotFound;

    auto con = connection.lock();
    if(con == nullptr) return eCodeError::FailedCheck;

    con->send("help!!!");

    return eCodeError::Ok;
}

exit_command::~exit_command() {}

eCodeError exit_command::execute(std::weak_ptr<tcp_server::tcp_connection> connection,
                           std::vector<std::string>& command_line)
{
    if(command_line.size() == 0) return eCodeError::CommandLineIsEmpty;

    if(command_line[0] != get_key_word()) return eCodeError::KeyWordIsNotFound;

    auto con = connection.lock();
    if(con == nullptr) return eCodeError::FailedCheck;

    auto srv = tcp_srv_.lock();
    if(srv == nullptr) return eCodeError::FailedCheck;

    con->stop();
    srv->erase_connection(con);

    return eCodeError::Ok;
}

get_files_command::~get_files_command() {}

eCodeError get_files_command::execute(std::weak_ptr<tcp_server::tcp_connection> connection, std::vector<std::string>& command_line)
{
    if(command_line.size() == 0) return eCodeError::CommandLineIsEmpty;

    if(command_line[0] != get_key_word()) return eCodeError::KeyWordIsNotFound;

    auto con = connection.lock();
    if(con == nullptr) return eCodeError::FailedCheck;

    auto files = app_.get_files("");

    std::string resp;
    if(files.has_value())
    {
        for(auto& f : files.value()) resp += f.first;
    }

    con->send(std::move(resp));

    return eCodeError::Ok;
}

//---------------------------------------------------------------

void protocol::add_command(std::unique_ptr<command>&& com)
{
    std::string key = com->get_key_word();
    command_map_.insert({key,std::move(com)});
}

void protocol::del_command(std::string key)
{
    auto it = command_map_.find(key);
    if(it == command_map_.end()) return;
    command_map_.erase(it);
}

eCodeError protocol::command_execute(std::weak_ptr<tcp_server::tcp_connection> connection, std::vector<std::string>& command_line)
{
    auto it = command_map_.find(command_line[0]);
    if(it == command_map_.end())
        return eCodeError::KeyWordIsNotFound;
    else
        return it->second->execute(connection, command_line);
}

//---------------------------------------------------------------

command_line_protocol::command_line_protocol(std::shared_ptr<logger::logger> logger)
    : logger::logable(logger)
{
    set_map_message_error();
}

command_line_protocol::command_line_protocol()
    : logger::logable(std::make_shared<logger::logger>(std::cout))
{
    set_map_message_error();
}

command_line_protocol::~command_line_protocol() {}

bool command_line_protocol::recieve(std::shared_ptr<tcp_server::tcp_connection> connection,
                                    std::vector<char>&& data)
{
    auto context_ = std::make_unique<command_line_protocol_context>(this->get_logger(), *this);
    context_->set_connection(connection);
    context_->set_data(std::move(data));
    context_->set_state(std::make_unique<splitting_into_tokens_state>());

    while(context_->handle());

    return true;
}

void command_line_protocol::set_map_message_error()
{
    map_message_error_.insert({eCodeError::CommandLineIsEmpty,"Command line is empty."});
    map_message_error_.insert({eCodeError::KeyWordIsNotFound,"Key word is not found."});
}

std::optional<std::string> command_line_protocol::mes_error(eCodeError code_error)
{
    auto it = map_message_error_.find(code_error);
    return it == map_message_error_.end() ? std::nullopt : std::optional(it->second);
}

//---------------------------------------------------------------

command_line_protocol_context::command_line_protocol_context(std::shared_ptr<logger::logger> logger_,
                                                             command_line_protocol& ref_clp)
    :logger::logable(logger_), ref_clp_(ref_clp) {}

bool command_line_protocol_context::handle()
{
    return state_->handle(*this);
}

//---------------------------------------------------------------

command_line_protocol_state::~command_line_protocol_state() {}

splitting_into_tokens_state::~splitting_into_tokens_state() {}

bool splitting_into_tokens_state::handle(command_line_protocol_context& context)
{
    std::stringstream lg;
    lg << "Command_line_protocol recieve " << context.data_.size() << " bytes.";
    context.log(lg.str()); lg.clear();

    std::string cl{context.data_.begin(),context.data_.end()};
    context.tokens_ = split(cl);

    if(context.tokens_.size() == 0)
    {
        lg << "Command line is empty.";
        context.log(lg.str());
        context.connection_->send(lg.str());
        return false;
    }
    context.set_state(std::make_unique<command_execute_state>());
    return true;
}

command_execute_state::~command_execute_state() {}

bool command_execute_state::handle(command_line_protocol_context& context)
{
    eCodeError code_error = context.ref_clp_.command_execute(context.connection_, context.tokens_);

    if(code_error != eCodeError::Ok)
    {
        std::stringstream lg;
        auto opt = context.ref_clp_.mes_error(code_error);
        if(opt)
        {
            lg << *opt;
            context.log(lg.str()); lg.clear();

            context.connection_->send(*opt);
        }
        else
        {
            lg << "Error command_line_protocol: " << static_cast<int>(code_error);
            context.log(lg.str());
        }
    }
    return false;
}




} // namespace tcp_server
