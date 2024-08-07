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
#include "monitoring.h"
#include "node.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <ecal/ecal.h>
#include <set>

class ProcessGraphFilter : public QWidget {
  Q_OBJECT

public:
  ProcessGraphFilter() = default;
  ProcessGraphFilter(Monitoring *monitor_);
  ~ProcessGraphFilter() = default;

  bool isInBlacklist(const std::string &id);
  bool isInBlacklist(const int &id);
  bool isInBlacklist(const eCAL::ProcessGraph::SProcessGraphEdge &edge);
  int getCentralProcess();

  void addToBlacklist(std::string entry);
  void removeFromBlacklist(std::string entry);

  // QT elements
  QComboBox *comboBox = new QComboBox();
  QLineEdit *addToBlacklistEdit = new QLineEdit();
  QPushButton *buttonAdd = new QPushButton("Add to filter");
  QLineEdit *removeFromBlacklistEdit = new QLineEdit();
  QPushButton *buttonRemove = new QPushButton("Remove from filter");
  QLabel *blacklistList = new QLabel();

public slots:
  void updateCentralProcess();
  void updateComboBox();

private:
  Monitoring *monitor;
  std::set<std::string> blacklist;
  int centralProcess = -1;
  QGridLayout *layout = new QGridLayout();

  void updateBlacklistList();
};