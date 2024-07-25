// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <ecal/ecal.h>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(TreeModel)

    TreeModel(const QStringList &headers, std::vector<eCAL::ProcessGraph::STopicTreeItem> &treeData,
              QObject *parent = nullptr);
    ~TreeModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = {}) override;
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = {}) override;
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = {}) override;
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = {}) override;

private:
    void setupModelData(const std::vector<eCAL::ProcessGraph::STopicTreeItem> &treeData);
    void insertProcess(size_t &pos, std::string direction, std::string currentTopic, const std::vector<eCAL::ProcessGraph::STopicTreeItem> &treeData);
    TreeItem *getItem(const QModelIndex &index) const;

    std::unique_ptr<TreeItem> rootItem;
};