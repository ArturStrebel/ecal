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

#include "filter.h"

ProcessGraphFilter::ProcessGraphFilter()
{
  QObject::connect(buttonAdd, &QPushButton::clicked, [=]() {
      blockedNames.insert(addToFilter->text().toStdString());
      addToFilter->clear();
    });
  QObject::connect(buttonRemove, &QPushButton::clicked, [=]() { 
      blockedNames.erase(removeFromFilter->text().toStdString());
      removeFromFilter->clear();
    });
}

std::string ProcessGraphFilter::getSelectedProcess()
{
  return selected_process;
}

bool ProcessGraphFilter::isInFilterList(const std::string& id)
{
  return std::find(blockedNames.begin(), blockedNames.end(), id) != blockedNames.end();
}

bool ProcessGraphFilter::isInFilterList(const int& id)
{
  return isInFilterList(std::to_string(id));
}

bool ProcessGraphFilter::isInFilterList(const eCAL::ProcessGraph::SProcessGraphEdge& edge)
{
  return (isInFilterList(edge.edgeID.first) || isInFilterList(edge.edgeID.first) || isInFilterList(edge.publisherName) || isInFilterList(edge.subscriberName));
}

bool ProcessGraphFilter::isInFilterList(const Edge* edge)
{
  return (isInFilterList(edge->sourceNode()->getId()) || isInFilterList(edge->destNode()->getId()) || isInFilterList(edge->sourceNode()->getName().toStdString()) || isInFilterList(edge->destNode()->getName().toStdString()));
}