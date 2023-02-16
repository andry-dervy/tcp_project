#include "request.h"

namespace request_client {

request::~request()
{
    log_("Destructor request");
}

void request::execute()
{
    try {
        connection_ = client_.create_connection();
        set_state(eStateRequest::ReadyToSend);
    }  catch (boost::system::system_error& e) {
        log_() << "request::execute called exeption: " << e.what() << std::endl;
        return;
    }

    connection_->data = data_;
    auto req = shared_from_this();
    client_.async_write_data(connection_,[req](tcp_client::connection_ptr&& c, const boost::system::error_code& err){
        if(err)
        {
            req->log_("Error on sending data: " + err.message() + '\n');
            return;
        }
        req->on_send();
        req->set_state(eStateRequest::WaitRead);
        req->client_.async_read_data_at_least(std::move(c),[req](tcp_client::connection_ptr&& c, const boost::system::error_code& err)
        {
            //if (req->timer_) req->timer_->cancel();
            if (err && err != boost::asio::error::eof) {
                req->log_("Client error on receive: " + err.message() + '\n');
                return;
            }
            //req->on_read(std::move(c->data));
            //req->set_state(eStateRequest::Done);
            req->log_("mes_code: " + std::to_string(c->data[0]) + " " + std::to_string(c->data[1]) + '\n');
            req->log_("mes_length: " + std::to_string(c->data[2]) + " " + std::to_string(c->data[3]) + " " +
                    std::to_string(c->data[4]) + " " + std::to_string(c->data[5]) + '\n');
            uint16_t mes_code = ntohs(static_cast<uint16_t>(c->data[0]) | static_cast<uint16_t>(c->data[1] << 8));
            uint32_t mes_length = ntohl(static_cast<uint32_t>(c->data[2]) |
                                        static_cast<uint32_t>(c->data[3] << 8) |
                                        static_cast<uint32_t>(c->data[4] << 16) |
                                        static_cast<uint32_t>(c->data[5] << 24) );
            req->log_("mes_length: " + std::to_string(mes_length) + '\n');
            req->client_.async_read_data_at_least(std::move(c),[req](tcp_client::connection_ptr&& c, const boost::system::error_code& err)
            {
                if (req->timer_) req->timer_->cancel();
                if (err && err != boost::asio::error::eof) {
                    req->log_("Client error on receive: " + err.message() + '\n');
                    return;
                }
                req->log_("Message data length: " + std::to_string(c->data.size()) + '\n');
                req->log_(std::string(c->data.begin(), c->data.end()));
                req->on_read(std::move(c->data));
                req->set_state(eStateRequest::Done);
            },
            1,
            mes_length);
        },
        1,
        sizeof (uint16_t) + sizeof (uint32_t));
    });

    timer_ = client_.run_delayed(boost::posix_time::seconds(5),
                        [req](){
        req->connection_->shutdown();
        req->on_timeout();
        req->set_state(eStateRequest::TimeOut);
    });
}

void request::set_state(eStateRequest state)
{
    std::lock_guard lock{mtx_state};
    state_ = state;
}

universe_request::~universe_request() {}

void universe_request::on_send() const
{
    out_ << "\nSend request: \"" << std::string(data_.begin(), data_.end()) << "\"..." << std::endl;
}

void universe_request::on_read(std::vector<char>&& data) const
{
    out_ << "Read respose: \"" << std::string(data.begin(), data.end()) << "\"..." << std::endl;
}

void universe_request::on_timeout() const
{
    out_ << "The waiting time has expired" << std::endl;
}

} // namespace request
