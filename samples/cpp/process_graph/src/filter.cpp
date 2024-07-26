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

ProcessGraphFilter::ProcessGraphFilter(Monitoring *monitor_) : monitor(monitor_) {

  layout->addWidget(comboBox, 0, 0);
  layout->addWidget(new QLabel("Central Process", this), 0, 1);
  layout->addWidget(addToBlackListEdit, 1, 0);
  layout->addWidget(buttonAdd, 1, 1);
  layout->addWidget(removeFromBlackListEdit, 2, 0);
  layout->addWidget(buttonRemove, 2, 1);
  layout->addWidget(blackListList, 3, 0, 1, 2);

  blackListList->setAlignment(Qt::AlignTop);
  setLayout(layout);

  for (const auto &item : monitor->getProcessGraph().topicTreeItems) {
    comboBox->addItem(QString::fromStdString(item.processName) +
                      " (PID: " + QString::number(item.processID) + ")");
  }

  show();

  QObject::connect(buttonAdd, &QPushButton::clicked, this, &ProcessGraphFilter::addToBlackList);
  QObject::connect(addToBlackListEdit, &QLineEdit::returnPressed, this,
                   &ProcessGraphFilter::addToBlackList);

  QObject::connect(buttonRemove, &QPushButton::clicked, this,
                   &ProcessGraphFilter::removeFromBlackList);
  QObject::connect(removeFromBlackListEdit, &QLineEdit::returnPressed, this,
                   &ProcessGraphFilter::removeFromBlackList);

  QObject::connect(monitor, &Monitoring::updateTopicTree, this,
                   &ProcessGraphFilter::updateComboBox);
  QObject::connect(comboBox, &QComboBox::currentIndexChanged, this,
                   &ProcessGraphFilter::updateCentralProcess);
}

void ProcessGraphFilter::updateComboBox() {

  auto newTopicTree = monitor->getProcessGraph().topicTreeItems;

  for (int index = 0; index < comboBox->count();) {
    bool found = false;
    for (auto treeItem : newTopicTree)
      if (treeItem.processName == comboBox->itemText(index).toStdString())
        found = true;
    if (found == false)
      comboBox->removeItem(index);
    else
      index++;
  }

  for (const auto &item : newTopicTree) {
    QString newItem = QString::fromStdString(item.processName) +
                      " (PID: " + QString::number(item.processID) + ")";
    if (comboBox->findText(newItem, Qt::MatchExactly) == -1)
      comboBox->addItem(newItem);
  }
}

void ProcessGraphFilter::updateCentralProcess() {
  std::string comboBoxText = comboBox->currentText().toStdString();
  std::size_t start = comboBoxText.find(": ");
  if (start != std::string::npos) {
    start += 2;
    std::size_t end = comboBoxText.find(")", start);
    if (end != std::string::npos) {
      centralProcess = std::stoi(comboBoxText.substr(start, end - start));
    }
  }
}

void ProcessGraphFilter::updateBlackListList() {
  QStringList qBlackList;
  std::transform(blackList.begin(), blackList.end(), std::back_inserter(qBlackList),
                 [](const std::string &str) { return QString::fromStdString(str); });
  blackListList->setText(qBlackList.join("\n"));
}

void ProcessGraphFilter::addToBlackList() {
  blackList.insert(addToBlackListEdit->text().toStdString());
  addToBlackListEdit->clear();
  updateBlackListList();
}

void ProcessGraphFilter::removeFromBlackList() {
  blackList.erase(removeFromBlackListEdit->text().toStdString());
  removeFromBlackListEdit->clear();
  updateBlackListList();
}

int ProcessGraphFilter::getCentralProcess() { return centralProcess; }

bool ProcessGraphFilter::isInBlackList(const std::string &id) {
  return std::find(blackList.begin(), blackList.end(), id) != blackList.end();
}

bool ProcessGraphFilter::isInBlackList(const int &id) { return isInBlackList(std::to_string(id)); }

bool ProcessGraphFilter::isInBlackList(const eCAL::ProcessGraph::SProcessGraphEdge &edge) {
  return (isInBlackList(edge.edgeID.first) || isInBlackList(edge.edgeID.first) ||
          isInBlackList(edge.publisherName) || isInBlackList(edge.subscriberName));
}