#include "api_client.h"
#include "tab.h"
#include "cassert"

namespace user {

api_client::api_client(tcp_client::tcp_client& client)
    : user_client(client)
{}

api_client::api_client(tcp_client::tcp_client& client, std::string address, int port)
    : user_client(client, address, port)
{
}

api_client::~api_client()
{
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
    QVector<QStringList> vec_for_view;
    for(const auto& m : v)
    {
        vec_for_view.push_back(QStringList() << m.at("name").c_str() << m.at("type").c_str());
    }
    model_->addDataForView(vec_for_view);
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

get_dirs_r::get_dirs_r(tcp_client::tcp_client &client, api_client &api)
    : request(client), api_(api), model_(nullptr)
{
}

void get_dirs_r::on_send() const
{
    log_() << "get_dirs_r is send." << std::endl;
}

void get_dirs_r::on_read(std::vector<char> &&data) const
{
    assert(model_ != nullptr);

    log_() << "get_dirs_r read data." << std::endl;

    protocol::get_list_dirs_answer ans;
    try {
        ans.from_sequence(data);
    }  catch (boost::archive::archive_exception& e) {
        log_() << "archive_exception: " << e.what() << std::endl;
        return;
    }
    auto v = ans.get_dirs();
    QVector<QStringList> vec_for_view;
    for(const auto& m : v)
    {
        vec_for_view.push_back(QStringList() << m.at("name").c_str() << m.at("type").c_str());
    }
    model_->addDataForView(vec_for_view);
}

void get_dirs_r::on_timeout() const
{
    log_() << "get_dirs_r stopped by the timeout." << std::endl;
}

void get_dirs_r::set_path(std::string &path)
{
    path_ = path;
}

void get_dirs_r::set_file_manager_model(FileManagerModel *model) {model_ = model;}

void get_dirs_r::convert_to_data()
{
    protocol::get_list_dirs_request req;
    req.set_path(path_);

    protocol::message mes;
    mes.code_.vUI16 = static_cast<uint16_t>(protocol::eTypeMessage::get_list_dirs_request);
    mes.data_ = req.to_sequence();
    mes.length_.vUI32 = mes.data_.size();

    set_data(mes.to_sequence());
}

} // namespace ui
