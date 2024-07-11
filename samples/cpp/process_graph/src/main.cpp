// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphwidget.h"
#include "mainwindow.h"
#include "edge.h"
#include "node.h"
#include "monitoring.h"
#include <ecal/ecal.h>
#include <iostream>
#include <map>

#include <QApplication>
#include <QTime>
#include <QMainWindow>
#include <QHBoxLayout>

static int g_mon_timing = 1000;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("eCAL Process Graph");

    // initialize eCAL core API
    eCAL::Initialize(argc, argv, "process_graph", eCAL::Init::All);

    // monitoring instance to store snapshot
    eCAL::Monitoring::SMonitoring monitoring;
    eCAL::ProcessGraph::SProcessGraph process_graph;

    // give eCAL some time, otherwise GetMonitoring will return nothing
    eCAL::Process::SleepMS(2 * g_mon_timing);

    QWidget *centralWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);

    Monitoring* monitor = new Monitoring();

    // Host View Graph
    GraphWidget *HostNetworkWindow = new GraphWidget(monitor, GraphWidget::ViewType::HostView, nullptr, "Host Network traffic");

    // Topic View
    MainWindow *topicTreeWindow = new MainWindow(monitor);

    // Process View Graph
    GraphWidget *ProcessGraphWindow = new GraphWidget(monitor, GraphWidget::ViewType::ProcessView, nullptr, "Process Graph");

    layout->addWidget(HostNetworkWindow);
    layout->addWidget(topicTreeWindow);
    layout->addWidget(ProcessGraphWindow);

    mainWindow.setCentralWidget(centralWidget);
    mainWindow.show();

    return app.exec();
}
