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

#include "node.h"
#include "edge.h"
#include <set>
#include <QPushButton>
#include <QLineEdit>
#include <ecal/ecal.h>

class ProcessGraphFilter
{
  public:
    ProcessGraphFilter();
    ~ProcessGraphFilter() = default;

    bool isInFilterList(const std::string& id);
    bool isInFilterList(const int& id);
    bool isInFilterList(const eCAL::ProcessGraph::SProcessGraphEdge& edge);
    bool isInFilterList(const Edge* edge);

    // QT elements
    QLineEdit *addToFilter = new QLineEdit();
    QPushButton *buttonAdd = new QPushButton("Add to filter");
    QLineEdit *removeFromFilter = new QLineEdit();
    QPushButton *buttonRemove = new QPushButton("Remove from filter");

  private:
    std::set<std::string> blockedNames;
};