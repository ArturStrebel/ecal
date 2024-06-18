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

#include "plugin_widget.h"

#include <QFrame>
#include <QLayout>
#include <QFont>
#include <ecal/ecal.h>
#include <filesystem>
#include <sstream>

#include <QDebug>

PluginWidget::PluginWidget(const QString& topic_name, const QString&, QWidget* parent): QWidget(parent),
  subscriber_(topic_name.toStdString()),
  last_message_publish_timestamp_(eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(-1))),
  new_msg_available_(false)
{
  ui_.setupUi(this);

  // Timestamp warning
  int label_height = ui_.publish_timestamp_warning_label->sizeHint().height();
  QPixmap warning_icon = QPixmap(":/ecalicons/WARNING").scaled(label_height, label_height, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  ui_.publish_timestamp_warning_label->setPixmap(warning_icon);
  ui_.publish_timestamp_warning_label->setVisible(false);

  // Setup frame
  QFrame* frame = new QFrame(this);
  frame->setFrameShape(QFrame::StyledPanel);
  frame->setFrameStyle(QFrame::Plain);
  QLayout* frame_layout = new QVBoxLayout(this);
  frame_layout->setContentsMargins(QMargins(0, 0, 0, 0));
  frame->setLayout(frame_layout);

  size_label_ = new QLabel(this);
  size_label_->setText("-- No debug info written, yet --");
  frame_layout->addWidget(size_label_);

  blob_text_edit_ = new QPlainTextEdit(this);
  blob_text_edit_->setFont(QFont("Courier New"));
  blob_text_edit_->setReadOnly(true);
  frame_layout->addWidget(blob_text_edit_);

  ui_.content_layout->addWidget(frame);

  process_id_ = eCAL::Process::GetProcessID();
  std::string filepath("");
  for (const auto& dir_entry : std::filesystem::directory_iterator(eCAL::Util::GeteCALLogPath())) 
  {
    // separate process ID from file name
    filepath = dir_entry.path().generic_string(); 
    size_t start = filepath.find_last_of("/") + 1;
    filepath = filepath.substr(start);
    size_t end = filepath.find_last_of(".");
    if ( end != std::string::npos )
        filepath = filepath.substr(0, end);
    std::istringstream tokenizer(filepath);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(tokenizer, token, '_'))
        tokens.push_back(token);

    // save log file name if current file is sought log file
    if( std::stoi(tokens.back()) == process_id_ ) {
      log_file_name_ = dir_entry.path().generic_string();
    break;
    // NOTE: There could be multiple files with same pID!
    // TODO: In case of multiple files -> take latest one (check timestamp)
    }
  }

  // Connect the eCAL Subscriber
  subscriber_.AddReceiveCallback(std::bind(&PluginWidget::ecalMessageReceivedCallback, this, std::placeholders::_2));
}

PluginWidget::~PluginWidget()
{
  subscriber_.RemReceiveCallback();
}

void PluginWidget::ecalMessageReceivedCallback(const struct eCAL::SReceiveCallbackData* callback_data)
{
  std::lock_guard<std::mutex> message_lock(message_mutex_);
  last_message_ = QByteArray(static_cast<char*>(callback_data->buf), callback_data->size);

  last_message_publish_timestamp_ = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(callback_data->time));

  // process_id_ = eCAL::Monitoring::SMonitoring.process[0].pid;
  new_msg_available_ = true;
}

void PluginWidget::onUpdate()
{
  //if (new_msg_available_) 
  {
    updateLogFileView();
    updatePublishTimeLabel();
  }
}

void PluginWidget::onResume()
{
  subscriber_.AddReceiveCallback(std::bind(&PluginWidget::ecalMessageReceivedCallback, this, std::placeholders::_2));
}

void PluginWidget::onPause()
{
  subscriber_.RemReceiveCallback();
}

void PluginWidget::updateLogFileView()
{
  std::lock_guard<std::mutex> message_lock(message_mutex_);

  size_label_->setText(tr(" Logfile path: ") + QString::fromStdString(log_file_name_));

  // write out log file
  std::ifstream log_file(log_file_name_);
  std::string line;
  int current_line_number = 0;
  while (std::getline(log_file, line)) {
    if(current_line_number++ > last_line_number_) { // if log file has new entries
      last_line_number_++;
      blob_text_edit_->appendPlainText(QString::fromStdString(line));
    }
  }
  log_file.close();
  new_msg_available_ = false;
}

void PluginWidget::updatePublishTimeLabel()
{
  eCAL::Time::ecal_clock::time_point publish_time = last_message_publish_timestamp_;

  if (publish_time < eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(0)))
    return;

  QString time_string;
  double seconds_since_epoch = std::chrono::duration_cast<std::chrono::duration<double>>(publish_time.time_since_epoch()).count();
  time_string = QString::number(seconds_since_epoch, 'f', 6) + " s";

  ui_.publish_timestamp_label->setText(time_string);
}

QWidget* PluginWidget::getWidget()
{
  return this;
}