// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "treemodel.h"
#include "treeitem.h"

using namespace Qt::StringLiterals;

//! [0]
TreeModel::TreeModel(const QStringList &headers, std::vector<eCAL::ProcessGraph::STopicTreeItem>& treeData, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVariantList rootData;
    for (const QString &header : headers)
        rootData << header;

    rootItem = std::make_unique<TreeItem>(rootData);
    setupModelData(treeData);
}
//! [0]

//! [1]
TreeModel::~TreeModel() = default;
//! [1]

//! [2]
int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rootItem->columnCount();
}
//! [2]

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    TreeItem *item = getItem(index);

    return item->data(index.column());
}

//! [3]
Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}
//! [3]

//! [4]
TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (auto *item = static_cast<TreeItem*>(index.internalPointer()))
            return item;
    }
    return rootItem.get();
}
//! [4]

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    return (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        ? rootItem->data(section) : QVariant{};
}

//! [5]
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return {};
//! [5]

//! [6]
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return {};

    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);
    return {};
}
//! [6]

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    beginInsertColumns(parent, position, position + columns - 1);
    const bool success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    rootItem->columnCount());
    endInsertRows();

    return success;
}

//! [7]
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    return (parentItem != rootItem.get() && parentItem != nullptr)
        ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}
//! [7]

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

//! [8]
int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;

    const TreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}
//! [8]

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void TreeModel::setupModelData(const std::vector<eCAL::ProcessGraph::STopicTreeItem>& treeData)
{
    for ( size_t i = 0; i < treeData.size(); ) 
    {
        rootItem->insertChildren(rootItem->childCount(), 1, rootItem->columnCount());

        auto topicItem = rootItem->child(rootItem->childCount()-1);
        std::string currentTopic = treeData[i].topicName;

        topicItem->setData(0, QString::fromStdString(currentTopic));
        topicItem->insertChildren(0, 2, rootItem->columnCount());

        while(treeData[i].topicName == currentTopic && treeData[i].direction == "publisher")
        {
            auto dirItem = topicItem->child(0);
            dirItem->setData(0, "Publisher");
            dirItem->insertChildren(dirItem->childCount(), 1, rootItem->columnCount());
            auto processItem = dirItem->child(dirItem->childCount() -1);
            processItem->setData(0, QString::fromStdString(treeData[i].processName));
            processItem->setData(1, QString::fromStdString(treeData[i].description));
            if (++i == treeData.size())
                break; 
        }

        if (i == treeData.size())
            break;

        while(treeData[i].topicName == currentTopic && treeData[i].direction == "subscriber")
        {
            auto dirItem = topicItem->child(1);
            dirItem->setData(0, "Subscriber");
            dirItem->insertChildren(dirItem->childCount(), 1, rootItem->columnCount());
            auto processItem = dirItem->child(dirItem->childCount() -1);
            processItem->setData(0, QString::fromStdString(treeData[i].processName));
            processItem->setData(1, QString::fromStdString(treeData[i].description));
            if (++i == treeData.size())
                break;     
        }
    }           
}