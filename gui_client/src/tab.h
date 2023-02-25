#pragma once

#include <QWidget>
#include <QTreeView>
#include <QStatusBar>
#include <QLabel>
#include "api_client.h"
#include "server_settings_dialog.h"

enum class eTypeTab
{
    FileManager,
};

class Tab : public QWidget
{
    Q_OBJECT
public:
    explicit Tab(QWidget *parent = nullptr);
    virtual ~Tab() = default;

    virtual bool isTypeTab(eTypeTab type) const;
//    virtual QString getExtention() const = 0;

protected:
    eTypeTab type_;

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) = 0;

public:
    eTypeTab getType() const;

signals:
    void closeSubWnd(eTypeTab type);
};

class CreatorTab: public QObject
{
public:
    CreatorTab(QObject* parent)
        :QObject(parent){}
    virtual ~CreatorTab() = default;
    virtual Tab* newTab(QWidget* parent) = 0;
    virtual Tab* openTab(QString& fileName, QWidget* parent) = 0;
    virtual bool saveTab(QString& fileName, Tab& tab) = 0;

    template<class T>
    static T* create(QWidget* parent) {
        return new T(parent);
    }
};

class TreeItem
{
public:
    explicit TreeItem(const QVector<QVariant>& data, TreeItem* parentItem = nullptr);
    ~TreeItem();

    void appendChild(TreeItem* child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parentItem();

private:
    QVector<TreeItem*> child_items_;
    QVector<QVariant> item_data_;
    TreeItem *parent_item_ptr_;
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(const QStringList &data, QObject *parent = nullptr);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    std::unique_ptr<TreeItem> root_item_ptr_;
};

class FileManagerModel: public TreeModel
{
    Q_OBJECT
public:
    FileManagerModel(const QStringList &data, QObject* parent = nullptr);
    ~FileManagerModel();
    void addDataForView(QVector<QStringList>& data);
private:
    mutable std::mutex mtx_set_data_;
};

class FileManagerTab : public Tab
{
    Q_OBJECT
public:
    FileManagerTab(QWidget* parent = nullptr);
    void set_getter_server_settings(std::shared_ptr<getter_server_settings> getter) {getter_ = getter;}
    void set_logger(std::shared_ptr<logger::file_logger> log_ptr) {log_ptr_ = log_ptr;}
    void run_client(std::string address, int port);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setWidget();

private:
    QTreeView* listView_;
    QLabel* statusBarLabel_;
    FileManagerModel* model_;
    std::shared_ptr<getter_server_settings> getter_;
    std::unique_ptr<user::api_client> api_client_ptr_;
    std::shared_ptr<logger::file_logger> log_ptr_;

public:
    void get_list_files(std::string& path);
    void get_list_dirs(std::string& path);
};
