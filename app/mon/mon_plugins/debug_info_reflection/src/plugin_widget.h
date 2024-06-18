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

#include <ecal/mon/plugin.h>
#include <ecal/ecal.h>

#include "ui_plugin_widget.h"

#include <QPlainTextEdit>
#include <QLabel>
#include <fstream>
#include "ecal_def.h"

#include <mutex>

class PluginWidget : public QWidget, public eCAL::mon::PluginWidgetInterface
{
  Q_OBJECT
public:
  PluginWidget(const QString& topic_name, const QString& topic_type, QWidget* parent = Q_NULLPTR);
  virtual ~PluginWidget();

  virtual QWidget* getWidget();

public slots:
  virtual void onUpdate();

  virtual void onResume();
  virtual void onPause();

protected:
  Ui::PluginWidget ui_;

private slots:
  void updateLogFileView();
  void updatePublishTimeLabel();

private:
  QLabel*         size_label_;
  QPlainTextEdit* blob_text_edit_;

  eCAL::CSubscriber                  subscriber_;
  QByteArray                         last_message_;
  eCAL::Time::ecal_clock::time_point last_message_publish_timestamp_;
  std::mutex                         message_mutex_;

  bool                               new_msg_available_;
  int                                last_line_number_ = -1;
  long long                          process_id_;
  std::string                        log_file_name_;

  void ecalMessageReceivedCallback(const struct eCAL::SReceiveCallbackData* callback_data);
};
