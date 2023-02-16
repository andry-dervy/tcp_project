#include "server_settings_dialog.h"
#include <QFormLayout>
#include <QEvent>
#include <QCloseEvent>

server_settings_dialog::server_settings_dialog(std::shared_ptr<server_settings>& server_settings_ptr,
                                               QWidget *parent)
    : QDialog(parent)
    , server_settings_ptr_(server_settings_ptr)
{
    installEventFilter(this);
    set_widgets();
}

server_settings_dialog::~server_settings_dialog()
{
}

void server_settings_dialog::set_widgets()
{
    le_server_address_ = new QLineEdit(this);
    le_server_port_ = new QLineEdit(this);

    setWindowTitle(tr("Server settings"));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("&Address:"), le_server_address_);
    formLayout->addRow(tr("&Port:"), le_server_port_);
    setLayout(formLayout);
}

void server_settings_dialog::load_settings(QSettings* settings)
{
    Q_ASSERT(settings != nullptr);

    settings_ = settings;
    le_server_address_->setText(settings_->value("server_settings/address").toString());
    le_server_port_->setText(settings_->value("server_settings/port").toString());
}

void server_settings_dialog::save_settings()
{
    if(settings_ == nullptr) return;

    settings_->setValue("server_settings/address", le_server_address_->text());
    settings_->setValue("server_settings/port", le_server_port_->text());

    server_settings_ptr_->setAddress(le_server_address_->text().toStdString());
    server_settings_ptr_->setPort(le_server_port_->text().toInt());
}

bool server_settings_dialog::eventFilter(QObject *object, QEvent *event)
{
    if (object == this && event->type() == QEvent::Close) {
        save_settings();
        return true;
    }
    return false;
}

std::string server_settings::getAddress() const
{
    return address_;
}

void server_settings::setAddress(const std::string& value)
{
    address_ = value;
}

uint16_t server_settings::getPort() const
{
    return port_;
}

void server_settings::setPort(const uint16_t& value)
{
    port_ = value;
}
