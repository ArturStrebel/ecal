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
  // layout->addWidget(addToBlacklistEdit, 1, 0);
  // layout->addWidget(buttonAdd, 1, 1);
  // layout->addWidget(removeFromBlacklistEdit, 2, 0);
  // layout->addWidget(buttonRemove, 2, 1);
  // layout->addWidget(blacklistList, 3, 0, 1, 2);

  blacklistList->setAlignment(Qt::AlignTop);
  setLayout(layout);

  for (const auto &item : monitor->getProcessGraph().topicTreeItems) {
    comboBox->addItem(QString::fromStdString(item.processName) +
                      " (PID: " + QString::number(item.processID) + ")");
  }
  comboBox->setCurrentIndex(-1);
  show();

  QObject::connect(buttonAdd, &QPushButton::clicked, this, [this]() {
    addToBlacklist(addToBlacklistEdit->text().toStdString());
    addToBlacklistEdit->clear();
    updateBlacklistList();
  });

  QObject::connect(addToBlacklistEdit, &QLineEdit::returnPressed, this, [this]() {
    addToBlacklist(addToBlacklistEdit->text().toStdString());
    addToBlacklistEdit->clear();
    updateBlacklistList();
  });

  QObject::connect(buttonRemove, &QPushButton::clicked, this, [this]() {
    removeFromBlacklist(removeFromBlacklistEdit->text().toStdString());
    removeFromBlacklistEdit->clear();
    updateBlacklistList();
  });

  QObject::connect(removeFromBlacklistEdit, &QLineEdit::returnPressed, [this]() {
    removeFromBlacklist(removeFromBlacklistEdit->text().toStdString());
    removeFromBlacklistEdit->clear();
    updateBlacklistList();
  });

  QObject::connect(monitor, &Monitoring::updateTopicTree, this,
                   &ProcessGraphFilter::updateComboBox);
  QObject::connect(comboBox, &QComboBox::currentIndexChanged, this,
                   &ProcessGraphFilter::updateCentralProcess);
}

void ProcessGraphFilter::updateComboBox() {

  auto newTopicTree = monitor->getProcessGraph().topicTreeItems;
  int centralProcessSave = centralProcess;
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
  centralProcess = centralProcessSave;
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

void ProcessGraphFilter::updateBlacklistList() {
  QStringList qBlacklist;
  std::transform(blacklist.begin(), blacklist.end(), std::back_inserter(qBlacklist),
                 [](const std::string &str) { return QString::fromStdString(str); });
  blacklistList->setText(qBlacklist.join("\n"));
}

void ProcessGraphFilter::addToBlacklist(std::string entry) {
  blacklist.insert(entry);
  addToBlacklistEdit->clear();
  updateBlacklistList();
}

void ProcessGraphFilter::removeFromBlacklist(std::string entry) {
  blacklist.erase(entry);
  removeFromBlacklistEdit->clear();
  updateBlacklistList();
}

int ProcessGraphFilter::getCentralProcess() {
  return centralProcess;
}

bool ProcessGraphFilter::isInBlacklist(const std::string &id) {
  return std::find(blacklist.begin(), blacklist.end(), id) != blacklist.end();
}

bool ProcessGraphFilter::isInBlacklist(const int &id) {
  return isInBlacklist(std::to_string(id));
}

bool ProcessGraphFilter::isInBlacklist(const eCAL::ProcessGraph::SProcessGraphEdge &edge) {
  return (isInBlacklist(edge.edgeID.first) || isInBlacklist(edge.edgeID.first) ||
          isInBlacklist(edge.publisherName) || isInBlacklist(edge.subscriberName));
}