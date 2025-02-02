// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "edge.h"
#include "filter.h"
#include "graphwidget.h"
#include "mainwindow.h"
#include "monitoring.h"
#include "node.h"
#include <ecal/ecal.h>
#include <iostream>
#include <map>

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <QTime>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QMainWindow mainWindow;
  mainWindow.setWindowTitle("eCAL Network Monitor");

  // initialize eCAL core API
  eCAL::Initialize(argc, argv, "monitoring", eCAL::Init::All);

  QWidget *centralWidget = new QWidget;
  QGridLayout *layout = new QGridLayout(centralWidget);
  QTabWidget *tabLayout = new QTabWidget();
  QGridLayout *subLayout = new QGridLayout();

  Monitoring *Monitor = new Monitoring();
  ProcessGraphFilter *filter = new ProcessGraphFilter(Monitor);
  QPushButton *PauseButton = new QPushButton("Pause");
  PauseButton->setCheckable(true);

  // TODO: Funktionen auf ihren Zugriff überprüfen, was sollte public sein, was
  // private?
  GraphWidget *HostTrafficView =
      new GraphWidget(Monitor, filter, PauseButton, GraphWidget::ViewType::HostView, nullptr,
                      "Host Network traffic");
  MainWindow *TopicTreeView = new MainWindow(Monitor, PauseButton);
  GraphWidget *ProcessGraphView = new GraphWidget(
      Monitor, filter, PauseButton, GraphWidget::ViewType::ProcessView, nullptr, "Process Graph");

  tabLayout->addTab(ProcessGraphView, "Process Graph");
  tabLayout->addTab(HostTrafficView, "Host Traffic");
  subLayout->addWidget(PauseButton, 0, 0);
  subLayout->addWidget(filter, 1, 0);
  layout->addWidget(tabLayout, 0, 0);
  layout->addLayout(subLayout, 1, 0);

  mainWindow.setCentralWidget(centralWidget);
  mainWindow.show();

  return app.exec();
}
