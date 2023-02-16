#include "mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include <iostream>

#include "../tcp_client_asio/user_client.h"
#include "api_client.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , settings_("app.ini", QSettings::IniFormat)
    , server_settings_dialog_(nullptr)
    , log_ptr_(std::make_shared<logger::file_logger>("log_error.txt"))
{
    resize(WIDTH, HEIGH);
    setMenuFile();
    setMdi();

    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setWidgets()
{

}

void MainWindow::setMdi()
{
    mdi = new QMdiArea(this);
    mdi->setViewMode(QMdiArea::ViewMode::TabbedView);
    connect(mdi, &QMdiArea::subWindowActivated, this, &MainWindow::subWindowActivated);
    setCentralWidget(mdi);
}

void MainWindow::setMenuFile()
{
    auto menuFile = new QMenu(tr("&File"),this);
    menuBar()->addMenu(menuFile);

    auto act = new QAction(tr("&Quit"),this);
    act->setShortcut(QKeySequence("Ctrl+Q"));
    menuFile->addAction(act);
    connect(act, &QAction::triggered, this, &MainWindow::quit);

    auto menuSettings = new QMenu(tr("&Settings"),this);
    menuBar()->addMenu(menuSettings);

    act = new QAction(tr("&Server settings"),this);
    menuSettings->addAction(act);
    connect(act, &QAction::triggered, this, &MainWindow::open_server_settings_dialog);

    auto menuConnections = new QMenu(tr("&Connections"),this);
    menuBar()->addMenu(menuConnections);

    actFileManagerTab = new QAction(tr("&Add tab client"),this);
    menuConnections->addAction(actFileManagerTab);
    connect(actFileManagerTab, &QAction::triggered, this, &MainWindow::add_tab_client);
}

void MainWindow::loadSettings()
{
    server_settings_ptr_ = std::make_shared<server_settings>();
    server_settings_ptr_->setAddress(settings_.value("server_settings/address").toString().toStdString());
    server_settings_ptr_->setPort(settings_.value("server_settings/port").toInt());

    log_ptr_->log("loadSettings: server_settings/address " + server_settings_ptr_->getAddress() +
                  " server_settings/port " + std::to_string(server_settings_ptr_->getPort()));
}

void MainWindow::saveSettings()
{
}

void MainWindow::quit()
{
    qApp->quit();
}

void MainWindow::open_server_settings_dialog()
{
    if(!server_settings_dialog_)
    {
        server_settings_dialog_ = new server_settings_dialog(server_settings_ptr_, this);
        server_settings_dialog_->load_settings(&settings_);
    }

    server_settings_dialog_->show();
}

void MainWindow::addSubWindow(Tab* tab)
{
    if(tab == nullptr) return;

    auto subwnd = mdi->addSubWindow(tab);
    subwnd->installEventFilter(tab);
    connect(tab,&Tab::closeSubWnd,this,&MainWindow::closeSubWnd);
    tab->show();
}

void MainWindow::add_tab_client()
{
    auto fileManager = CreatorTab::create<FileManagerTab>(this);
    fileManager->set_getter_server_settings(server_settings_ptr_);
    fileManager->set_logger(log_ptr_);
    fileManager->run_client(server_settings_ptr_->getAddress(), server_settings_ptr_->getPort());
    addSubWindow(fileManager);

    std::string path = "";
    //fileManager->get_list_files(path);

}

void MainWindow::subWindowActivated()
{

}

void MainWindow::closeSubWnd(eTypeTab type)
{
    Q_UNUSED(type);
}


