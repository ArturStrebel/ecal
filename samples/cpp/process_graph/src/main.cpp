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


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // initialize eCAL core API
    eCAL::Initialize(argc, argv, "monitoring", eCAL::Init::All);

    // eCAL::Monitoring::SMonitoring monitoring;
    eCAL::ProcessGraph::SProcessGraph process_graph;

//     eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::All);
//     process_graph = eCAL::ProcessGraph::GetProcessGraph(monitoring);

    QWidget *centralWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);

    Monitoring* monitor = new Monitoring();

    // Host View Graph
    GraphWidget *widget1 = new GraphWidget(monitor, GraphWidget::ViewType::HostView, nullptr, "Host Network traffic");

    // Topic View
    MainWindow *widget2 = new MainWindow(monitor);

    // Process View Graph
    GraphWidget *widget3 = new GraphWidget(monitor, GraphWidget::ViewType::ProcessView, nullptr, "Process Graph", "ACU Process");

    layout->addWidget(widget1);
    layout->addWidget(widget2);
    layout->addWidget(widget3);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.show();

    return app.exec();
}
