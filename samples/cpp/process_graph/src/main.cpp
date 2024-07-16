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

    QWidget *centralWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);

    Monitoring* monitor = new Monitoring();
    GraphWidget *HostTrafficView = new GraphWidget(monitor, GraphWidget::ViewType::HostView, nullptr, "Host Network traffic");
    MainWindow *TopicTreeView = new MainWindow(monitor);
    GraphWidget *ProcessGraphView = new GraphWidget(monitor, GraphWidget::ViewType::ProcessView, nullptr, "Process Graph");

    layout->addWidget(HostTrafficView);
    layout->addWidget(TopicTreeView);
    layout->addWidget(ProcessGraphView);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.show();

    return app.exec();
}
