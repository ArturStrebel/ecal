// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include "treeitem.h"

TreeItem::TreeItem(QVariantList data, TreeItem *parent)
    : itemData(std::move(data)), m_parentItem(parent)
{
}

TreeItem *TreeItem::child(int number)
{
    return (number >= 0 && number < childCount())
               ? m_childItems.at(number).get()
               : nullptr;
}

int TreeItem::childCount() const
{
    return int(m_childItems.size());
}

int TreeItem::row() const
{
    if (!m_parentItem)
        return 0;
    const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                 [this](const std::unique_ptr<TreeItem> &treeItem)
                                 {
                                     return treeItem.get() == this;
                                 });

    if (it != m_parentItem->m_childItems.cend())
        return std::distance(m_parentItem->m_childItems.cbegin(), it);
    Q_ASSERT(false); // should not happen
    return -1;
}

int TreeItem::columnCount() const
{
    return int(itemData.count());
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > qsizetype(m_childItems.size()))
        return false;

    for (int row = 0; row < count; ++row)
    {
        QVariantList data(columns);
        m_childItems.insert(m_childItems.cbegin() + position,
                            std::make_unique<TreeItem>(data, this));
    }

    return true;
}

bool TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    for (auto &child : std::as_const(m_childItems))
        child->insertColumns(position, columns);

    return true;
}

TreeItem *TreeItem::parent()
{
    return m_parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > qsizetype(m_childItems.size()))
        return false;

    for (int row = 0; row < count; ++row)
        m_childItems.erase(m_childItems.cbegin() + position);

    return true;
}

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    for (auto &child : std::as_const(m_childItems))
        child->removeColumns(position, columns);

    return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}
