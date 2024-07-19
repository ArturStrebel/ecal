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
#include <QGridLayout>
#include <QPushButton>


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // initialize eCAL core API
    eCAL::Initialize(argc, argv, "monitoring", eCAL::Init::All);

    QWidget *centralWidget = new QWidget;
    QGridLayout *layout = new QGridLayout(centralWidget);

    Monitoring* Monitor = new Monitoring();
    QPushButton *PauseButton = new QPushButton("Pause"); 
    PauseButton->setCheckable(true);   
    GraphWidget *HostTrafficView = new GraphWidget(Monitor, PauseButton, GraphWidget::ViewType::HostView, nullptr, "Host Network traffic");
    MainWindow *TopicTreeView = new MainWindow(Monitor, PauseButton);
    GraphWidget *ProcessGraphView = new GraphWidget(Monitor, PauseButton, GraphWidget::ViewType::ProcessView, nullptr, "Process Graph");

    layout->addWidget(HostTrafficView,1,1);
    layout->addWidget(TopicTreeView,0,0);
    layout->addWidget(ProcessGraphView,0,1);
    layout->addWidget(PauseButton,1,0);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.show();

    return app.exec();
}
