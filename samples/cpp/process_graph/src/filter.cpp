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

#include <QGridLayout>

ProcessGraphFilter::ProcessGraphFilter()
{
  QGridLayout *layout = new QGridLayout();

  layout->addWidget(setCentralProcessEdit,0,0);    
  layout->addWidget(buttonSet,0,1);
  layout->addWidget(addToBlackListEdit,1,0);    
  layout->addWidget(buttonAdd,1,1);
  layout->addWidget(removeFromBlackListEdit,2,0);    
  layout->addWidget(buttonRemove,2,1);
  setLayout(layout);

  QObject::connect(buttonSet, &QPushButton::clicked, this, &ProcessGraphFilter::setCentralProcess);
  QObject::connect(setCentralProcessEdit, &QLineEdit::returnPressed, this, &ProcessGraphFilter::setCentralProcess);

  QObject::connect(buttonAdd, &QPushButton::clicked, this, &ProcessGraphFilter::addToBlackList);
  QObject::connect(addToBlackListEdit, &QLineEdit::returnPressed, this, &ProcessGraphFilter::addToBlackList);

  QObject::connect(buttonRemove, &QPushButton::clicked, this, &ProcessGraphFilter::removeFromBlackList);
  QObject::connect(removeFromBlackListEdit, &QLineEdit::returnPressed, this, &ProcessGraphFilter::removeFromBlackList);

  show();
}

void ProcessGraphFilter::setCentralProcess()
{
  centralProcess = setCentralProcessEdit->text().toStdString();
}

void ProcessGraphFilter::addToBlackList()
{
  blackList.insert(addToBlackListEdit->text().toStdString());
  addToBlackListEdit->clear();
}

void ProcessGraphFilter::removeFromBlackList()
{
  blackList.erase(removeFromBlackListEdit->text().toStdString());
  removeFromBlackListEdit->clear();
}

std::string ProcessGraphFilter::getCentralProcess()
{
  return centralProcess;
}

bool ProcessGraphFilter::isInFilterList(const std::string& id)
{
  return std::find(blackList.begin(), blackList.end(), id) != blackList.end();
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