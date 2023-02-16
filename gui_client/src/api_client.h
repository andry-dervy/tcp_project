#pragma once

#include "../tcp_client_asio/user_client.h"
#include "../tcp_client_asio/request.h"
#include "../tcp_server_asio/requestprotocol.h"

class FileManagerModel;

namespace user {

class api_client;

class get_files_r: public request_client::request
{
public:
    get_files_r(tcp_client::tcp_client& client, api_client& api);
    ~get_files_r() override = default;

    void on_send() const override;
    void on_read(std::vector<char>&& data) const override;
    void on_timeout() const override;

    void set_path(std::string& path);
    void set_file_manager_model(FileManagerModel* model) {model_ = model;}
    void convert_to_data();

private:
    api_client& api_;
    FileManagerModel* model_;
    std::string path_;
    std::vector<std::string> vec_files_;
};

class api_client: public user_client
{
public:
    api_client(std::shared_ptr<logger::logger> logger);
    ~api_client() override;

    void start() override;
    void stop() override;
};

} // namespace user
