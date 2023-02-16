#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QAction>

#include "server_settings_dialog.h"
#include "tab.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

    const int WIDTH = 500;
    const int HEIGH = 500;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    QSettings settings_;
    server_settings_dialog* server_settings_dialog_;
    std::shared_ptr<server_settings> server_settings_ptr_;
    std::shared_ptr<logger::file_logger> log_ptr_;

    void setWidgets();
    void setMenuFile();
    void setMdi();

    void loadSettings();
    void saveSettings();

    void addSubWindow(Tab* tab);

private:
    QMdiArea* mdi;
    QAction* actFileManagerTab;

private slots:
    void quit();
    void open_server_settings_dialog();
    void add_tab_client();
    void subWindowActivated();
    void closeSubWnd(eTypeTab type);
};

