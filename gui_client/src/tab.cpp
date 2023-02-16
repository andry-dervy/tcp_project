#include "tab.h"
#include <QVBoxLayout>
#include <fstream>

//-----------------------------------------------------------------

Tab::Tab(QWidget *parent) : QWidget(parent)
{
}

bool Tab::isTypeTab(eTypeTab type) const
{
    return type == type_ ? true : false;
}

eTypeTab Tab::getType() const
{
    return type_;
}

//-----------------------------------------------------------------

TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent_item)
    : item_data_(data), parent_item_ptr_(parent_item)
{
}

TreeItem::~TreeItem()
{
    qDeleteAll(child_items_);
}

void TreeItem::appendChild(TreeItem *child)
{
    assert(child != nullptr);
    child_items_.append(child);
}

TreeItem *TreeItem::child(int row)
{
    if(row < 0 || row >= child_items_.size())
        return nullptr;
    return child_items_[row];
}

int TreeItem::childCount() const
{
    return child_items_.size();
}

int TreeItem::columnCount() const
{
    return item_data_.size();
}

QVariant TreeItem::data(int column) const
{
    if(column < 0 || column >= item_data_.size())
        return QVariant();
    return item_data_[column];
}

int TreeItem::row() const
{
    if(parent_item_ptr_)
        return parent_item_ptr_->child_items_.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

TreeItem* TreeItem::parentItem()
{
    return parent_item_ptr_;
}

TreeModel::TreeModel(const QStringList &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> vec(data.size());
    int i = 0;
    for(auto& a : vec) a = data[i++];
    root_item_ptr_ = std::make_unique<TreeItem>(vec);
}

TreeModel::~TreeModel()
{
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();
    switch(role)
    {
    case Qt::DisplayRole:
        auto item = static_cast<TreeItem*>(index.internalPointer());
        if(item == nullptr) return QVariant();
        int col = index.column();
        return item->data(col);
        break;
    }
    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return root_item_ptr_->data(section);
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = root_item_ptr_.get();
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == root_item_ptr_.get())
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = root_item_ptr_.get();
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    return root_item_ptr_->columnCount();
}

FileManagerModel::FileManagerModel(const QStringList &data, QObject* parent)
    : TreeModel(data, parent)
{}

FileManagerModel::~FileManagerModel() {}

void FileManagerModel::addDataForView(QVector<QStringList>& data, int row, int col)
{
    std::lock_guard lock{mtx_set_data_};

    beginResetModel();
    for(const auto& d: data)
    {
        QVector<QVariant> v;
        for(const auto& s: d) v.append(s);
        root_item_ptr_->appendChild(new TreeItem(v, root_item_ptr_.get()));
    }
    endResetModel();
}

//-----------------------------------------------------------------

FileManagerTab::FileManagerTab(QWidget* parent)
    : Tab(parent)
    , api_client_ptr_(nullptr)
{
    type_ = eTypeTab::FileManager;
    setWidget();
}

bool FileManagerTab::eventFilter(QObject* obj, QEvent* event)
{
    return false;
}

void FileManagerTab::setWidget()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    listView_ = new QTreeView(this);
    layout->addWidget(listView_);
    setWindowTitle(tr("Список файлов"));
    listView_->installEventFilter(this);
    auto statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);
    statusBarLabel_ = new QLabel(this);
    statusBar->addWidget(statusBarLabel_);

    model_ = new FileManagerModel({"Название", "Тип"});

    QVector<QStringList> v;
    v.append(QStringList() << ".." << "");
    model_->addDataForView(v,0,0);

    listView_->setModel(model_);
}

void FileManagerTab::get_list_files(std::string& path)
{
    assert(api_client_ptr_ != nullptr);

    try
    {
        auto req_get_files = api_client_ptr_->make_request<user::get_files_r>(*api_client_ptr_);
        req_get_files->set_path(path);
        req_get_files->set_file_manager_model(model_);
        req_get_files->convert_to_data();
        api_client_ptr_->add_request(req_get_files);
    }
    catch (std::exception& e)
    {
        statusBarLabel_->setText(e.what());
    }
    catch (...)
    {
        statusBarLabel_->setText("Неожиданное исключение.");
    }
}

void FileManagerTab::run_client(std::string address, int port)
{
    assert(log_ptr_ != nullptr);

    log_ptr_->log("FileManagerTab::run_client: address = " + address +
             " port = " + std::to_string(port));

    try
    {
        api_client_ptr_ = std::make_unique<user::api_client>(log_ptr_);
        auto client = std::make_unique<tcp_client::tcp_client>(log_ptr_);
        client->set_address(address);
        client->set_port(port);
        api_client_ptr_->set_tcp_client(std::move(client));
        api_client_ptr_->start();
        statusBarLabel_->setText(tr("Client is started."));
    }
    catch (std::exception& e)
    {
        log_ptr_->log(e.what());
        statusBarLabel_->setText(e.what());
    }
    catch (...)
    {
        log_ptr_->log("Неожиданное исключение.");
        statusBarLabel_->setText("Неожиданное исключение.");
    }
}

//-----------------------------------------------------------------




