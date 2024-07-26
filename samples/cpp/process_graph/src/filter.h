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

/**
 * @file   filter.h
 * @brief  eCAL filter class for the process graph tool
 **/

#pragma once

#include "edge.h"
#include "node.h"
#include <QLineEdit>
#include <QPushButton>
#include <ecal/ecal.h>
#include <set>

class ProcessGraphFilter : public QWidget {
  Q_OBJECT

public:
  ProcessGraphFilter();
  ~ProcessGraphFilter() = default;

  bool isInBlackList(const std::string &id);
  bool isInBlackList(const int &id);
  bool isInBlackList(const eCAL::ProcessGraph::SProcessGraphEdge &edge);
  std::string getCentralProcess();

  // QT elements
  QLineEdit *setCentralProcessEdit = new QLineEdit();
  QPushButton *buttonSet = new QPushButton("Set as central process");
  QLineEdit *addToBlackListEdit = new QLineEdit();
  QPushButton *buttonAdd = new QPushButton("Add to filter");
  QLineEdit *removeFromBlackListEdit = new QLineEdit();
  QPushButton *buttonRemove = new QPushButton("Remove from filter");

public slots:
  void addToBlackList();
  void removeFromBlackList();
  void setCentralProcess();

private:
  std::set<std::string> blackList;
  std::string centralProcess = "";
};