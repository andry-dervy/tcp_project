#include "requestprotocol.h"
#include <boost/exception/all.hpp>
#include <sstream>

namespace protocol {

std::shared_ptr<protocol> create_requestprotocol(std::shared_ptr<tcp_server::tcp_server>& srv,
                                                       std::shared_ptr<app::file_manager_app>& app)
{
    auto prtcl = std::make_shared<request_protocol>(app, srv->get_logger());

//    prtcl->add_command(std::make_unique<exit_command>(srv, srv->get_logger()));
//    prtcl->add_command(std::make_unique<help_command>(srv->get_logger()));
//    prtcl->add_command(std::make_unique<get_files_command>(*app,srv->get_logger()));

    return std::move(prtcl);
}
//---------------------------------------------------------------
std::vector<char> request::to_sequence()
{
    std::stringstream ss;
    boost::archive::text_oarchive oa{ss};
    oa << this;
    auto str = ss.str();
    return std::vector<char>(str.begin(), str.end());
}

void request::from_sequence(const std::vector<char>& sequential)
{
    std::stringstream ss { std::string(sequential.begin(),sequential.end()) };
    boost::archive::text_iarchive ia{ss};
    request* req;
    ia >> req;
    type_request_ = req->type_request_;
    delete req;
}

std::string request::type_request() const
{
    return type_request_;
}

void request::set_type_request(const std::string& type_request)
{
    type_request_ = type_request;
}

std::vector<char> get_list_files_request::to_sequence()
{
    std::stringstream ss;
    boost::archive::text_oarchive oa{ss};
    oa << this;
    auto str = ss.str();
    return std::vector<char>(str.begin(), str.end());
}

void get_list_files_request::from_sequence(const std::vector<char>& sequential)
{
    std::stringstream ss {std::string(sequential.begin(),sequential.end())};

    boost::archive::text_iarchive ia{ss};
    get_list_files_request* req = nullptr;
    try {
        ia >> req;
    }  catch (boost::archive::archive_exception& e) {
        if(req) delete req;
        throw e;
    }
    path_ = req->path_;
    delete req;
}

std::string get_list_files_request::path() const
{
    return path_;
}

void get_list_files_request::set_path(const std::string& path)
{
    path_ = path;
}
//---------------------------------------------------------------
app::eError answer::error() const
{
    return error_;
}

void answer::set_error(const app::eError& error)
{
    error_ = error;
}

std::string answer::error_string() const
{
    return error_string_;
}

void answer::set_error_string(const std::string& error_string)
{
    error_string_ = error_string;
}

std::vector<char> get_list_files_answer::to_sequence()
{
    std::stringstream ss;
    boost::archive::text_oarchive oa{ss};
    oa << this;
    auto str = ss.str();
    return std::vector<char>(str.begin(), str.end());
}

void get_list_files_answer::from_sequence(const std::vector<char>& sequential)
{
    std::stringstream ss { std::string(sequential.begin(),sequential.end()) };
    boost::archive::text_iarchive ia{ss};

    get_list_files_answer* ans = nullptr;
    try {
        ia >> ans;
    }  catch (boost::archive::archive_exception& e) {
        if(ans) delete ans;
        throw e;
    }
    files_ = ans->files_;
    delete ans;
}

std::vector<std::string> get_list_files_answer::files() const
{
    return files_;
}

void get_list_files_answer::set_files(const std::vector<std::string>& files)
{
    files_ = files;
}

std::vector<std::string> get_list_files_answer::get_files() const
{
    return files_;
}
//---------------------------------------------------------------
std::optional<std::vector<char>> get_list_files_executor::execute(std::vector<char>& sequential)
{
    get_list_files_request req;

    try {
        req.from_sequence(sequential);
    } catch (boost::archive::archive_exception& e) {
        log("get_list_files_executor::execute: " + std::string(e.what()));
        return std::nullopt;
    }
    auto list_files = fm_app_.get_files(req.path());
    log("number files: " + std::to_string(list_files.size()));
    get_list_files_answer ans;
    ans.set_files(list_files);
    ans.set_error(fm_app_.get_error());
    ans.set_error_string(fm_app_.get_error_string());

    message mes;
    mes.code_.vUI16 = static_cast<uint16_t>(eTypeMessage::get_list_files_answer);
    mes.data_ = ans.to_sequence();
    mes.length_.vUI32 = mes.data_.size();

    return std::make_optional(mes.to_sequence());
}
//---------------------------------------------------------------
request_protocol::request_protocol(std::shared_ptr<app::file_manager_app>& app, std::shared_ptr<logger::logger> logger)
    : logger::logable(logger), app_(app)
{
    type_protocol = eTypeProtocol::Request;
    fill_request_map();
}

request_protocol::request_protocol(std::shared_ptr<logger::logger> logger)
    : logger::logable(logger)
{}

bool request_protocol::recieve(
        std::shared_ptr<tcp_server::tcp_connection> connection,
        std::vector<char>&& data)
{
    enum class eState
    {
        header_code,
        header_length,
        data,
    };

    static std::deque<char> q;
    static std::unique_ptr<message> mes_ptr {nullptr};
    static eState state = eState::header_code;

    q.insert(q.end(), data.begin(), data.end());

    bool exit = false;
    while(!exit)
    {
        switch (state) {
        case eState::header_code:
            if(q.size() >= sizeof(message::code_))
            {
                mes_ptr = std::make_unique<message>();

                ui16 code;
                code.b1 = q.front(); q.pop_front();
                code.b2 = q.front(); q.pop_front();

                mes_ptr->code_.vUI16 = ntohs(code.vUI16);

                state = eState::header_length;

                log("header_code:" + std::to_string(mes_ptr->code_.vUI16));
            }
            else exit = true;
            break;
        case eState::header_length:
            if(q.size() >= sizeof(message::length_))
            {
                ui32 length;
                length.b1 = q.front(); q.pop_front();
                length.b2 = q.front(); q.pop_front();
                length.b3 = q.front(); q.pop_front();
                length.b4 = q.front(); q.pop_front();

                mes_ptr->length_.vUI32 = ntohl(length.vUI32);

                state = eState::data;
                log("header_length:" + std::to_string(mes_ptr->length_.vUI32));
            }
            else exit = true;
            break;
        case eState::data:
            if(q.size() >= mes_ptr->length_.vUI32)
            {
                state = eState::header_code;

                mes_ptr->data_.resize(mes_ptr->length_.vUI32);

                mes_ptr->data_.insert(mes_ptr->data_.begin(),
                                     q.begin(),
                                     q.begin()+mes_ptr->length_.vUI32);
                q.erase(q.begin(),
                        q.begin()+mes_ptr->length_.vUI32);

                log(std::string(mes_ptr->data_.begin(),mes_ptr->data_.end()));

                auto it_ex = executors_.find(static_cast<eTypeMessage>(mes_ptr->code_.vUI16));
                if(it_ex == executors_.end())
                    break;
                auto& ex = it_ex->second;

                auto opt = ex->execute(mes_ptr->data_);

                if(opt.has_value()) connection->send(std::move(*opt));
                q.clear();
            }
            else exit = true;
            break;
        }
    }
    return false;
}

void request_protocol::fill_request_map()
{
    executors_.clear();
    // add get_list_files request executor
    {
        auto ptr = std::make_unique<get_list_files_executor>(*app_, get_logger());
        executors_.insert({eTypeMessage::get_list_files_request, std::move(ptr)});
    }
}

void request_protocol::set_app(std::shared_ptr<app::file_manager_app>& app)
{
    app_ = app;
}

std::string get_list_files_key::get_key_string() const
{
    return key_string_;
}

} // namespace protocol
