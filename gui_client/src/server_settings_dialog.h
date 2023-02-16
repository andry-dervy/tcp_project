#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSettings>
#include <string>
#include <memory>

class getter_server_settings
{
public:
    virtual ~getter_server_settings() = default;
    virtual std::string getAddress() const = 0;
    virtual uint16_t getPort() const = 0;
};

class server_settings: public getter_server_settings
{
public:
    std::string getAddress() const override;
    void setAddress(const std::string& value);

    uint16_t getPort() const override;
    void setPort(const uint16_t& value);

private:
    std::string address_;
    uint16_t port_;
};

class server_settings_dialog : public QDialog
{
    Q_OBJECT
public:
    explicit server_settings_dialog(std::shared_ptr<server_settings>& server_settings_ptr,
                                    QWidget *parent = nullptr);
    ~server_settings_dialog();
    void load_settings(QSettings* settings);

private:
    void set_widgets();
    void save_settings();

private:
    QLineEdit* le_server_address_;
    QLineEdit* le_server_port_;

    QSettings* settings_;
    std::shared_ptr<server_settings> server_settings_ptr_;

protected:
     bool eventFilter(QObject* object, QEvent* event);

signals:

};

