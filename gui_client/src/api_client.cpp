#include "api_client.h"
#include "tab.h"
#include "cassert"

namespace user {

api_client::api_client(std::shared_ptr<logger::logger> logger)
    : user_client(logger)
{}

api_client::~api_client()
{
}

void api_client::start()
{
    start_thread_tcp_client();
}

void api_client::stop()
{
    client_->stop();
}

get_files_r::get_files_r(tcp_client::tcp_client& client, api_client& api)
    : request(client), api_(api), model_(nullptr)
{
}

void get_files_r::on_send() const
{
    log_() << "get_files_r is send." << std::endl;
}

void get_files_r::on_read(std::vector<char>&& data) const
{
    assert(model_ != nullptr);

    log_() << "get_files_r read data." << std::endl;

    protocol::get_list_files_answer ans;
    try {
        ans.from_sequence(data);
    }  catch (boost::archive::archive_exception& e) {
        log_() << "archive_exception: " << e.what() << std::endl;
        return;
    }
    auto v = ans.get_files();
    QVector<QString> vec_for_view;
    for(const auto& str : v)
    {
        vec_for_view.push_back(QString(str.c_str()));
        //QVector<QString> v {QString(str.c_str()), ""};

    }
    //model_->setDataForView(vec_for_view);
}

void get_files_r::on_timeout() const
{
    log_() << "get_files_r stopped by the timeout." << std::endl;
}

void get_files_r::set_path(std::string& path)
{
    path_ = path;
}

void get_files_r::convert_to_data()
{
    protocol::get_list_files_request req;
    req.set_path(path_);

    protocol::message mes;
    mes.code_.vUI16 = static_cast<uint16_t>(protocol::eTypeMessage::get_list_files_request);
    mes.data_ = req.to_sequence();
    mes.length_.vUI32 = mes.data_.size();

    set_data(mes.to_sequence());
}

} // namespace ui
