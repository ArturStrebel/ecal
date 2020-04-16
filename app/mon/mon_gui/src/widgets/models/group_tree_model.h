/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#pragma once

#include <CustomQt/QAbstractTreeModel.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "group_tree_item.h"

#include <QMap>
#include <QVector>
#include <QPair>

class GroupTreeModel : public QAbstractTreeModel
{
  Q_OBJECT

public:
  GroupTreeModel(const QVector<int>& group_by_columns, QObject *parent = nullptr);
  ~GroupTreeModel();

  void insertItemIntoGroups(QAbstractTreeItem* item);
  void removeItemFromGroups(QAbstractTreeItem* item, bool remove_empty_groups = true);

  void setGroupByColumns(const QVector<int>& group_by_columns);

  void setGroupColumnHeader(const QVariant& header);
  QVariant groupColumnHeader() const;

  QVector<int> groupByColumns() const;

  QVector<QPair<int, QString>> getTreeItemColumnNameMapping() const;

  virtual void monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb) = 0;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

  virtual int groupColumn() const = 0;

private:
  QMap<QVariant, GroupTreeItem*> group_map_;                                    /*< group_identifier -> TreeItem mapping*/
  QList<QAbstractTreeItem*> items_list_;

  QVariant group_column_header_;

  QVector<int> group_by_columns_;
};
