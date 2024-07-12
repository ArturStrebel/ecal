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

    // take snapshot :-)
    eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::All);
    // process_graph = eCAL::ProcessGraph::GetProcessGraph(monitoring);

    process_graph.hostEdges = {
        {true, std::make_pair(1,1), "HPC 1", "EDGE 1", 24.34},
        {true, std::make_pair(2,1), "EDGE 1", "HPC 1", 5.0},
        {true, std::make_pair(3,1), "HPC 1", "HPC 1", 1.34}
    };

    process_graph.processEdges = {
        {true, std::make_pair(1,1), "ACU Process", "sub1", "Topic 2", 5.0},
        {true, std::make_pair(2,1), "ACU Process", "sub2", "Topic 3", 3.2},
        {true, std::make_pair(3,1), "ACU Process", "sub3", "Topic 4", 0.01},
        {true, std::make_pair(4,1), "Camera", "ACU Process", "Rear_Camera", 9.8}
    };

    process_graph.topicTreeItems = {
        {true, 1, "T1", "Publisher", "pub1", "Important"},
        {true, 2, "T1", "Publisher", "pub2", "stuff"},
        {true, 3, "T1", "Subscriber", "sub1", "goes"},
        {true, 4, "T2", "Publisher", "pub1", "here"}
    };

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
