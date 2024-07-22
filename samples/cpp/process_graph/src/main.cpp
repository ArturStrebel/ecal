// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphwidget.h"
#include "mainwindow.h"
#include "edge.h"
#include "node.h"
#include "monitoring.h"
#include "filter.h"
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
    QGridLayout *subLayout = new QGridLayout();

    Monitoring* Monitor = new Monitoring();
    ProcessGraphFilter* filter = new ProcessGraphFilter();
    QPushButton *PauseButton = new QPushButton("Pause"); 
    PauseButton->setCheckable(true);

    GraphWidget *HostTrafficView = new GraphWidget(Monitor, filter, PauseButton, GraphWidget::ViewType::HostView, nullptr, "Host Network traffic");
    MainWindow *TopicTreeView = new MainWindow(Monitor, PauseButton);
    GraphWidget *ProcessGraphView = new GraphWidget(Monitor, filter, PauseButton, GraphWidget::ViewType::ProcessView, nullptr, "Process Graph");

    layout->addWidget(HostTrafficView,1,1);
    layout->addWidget(TopicTreeView,0,0);
    layout->addWidget(ProcessGraphView,0,1);
    
    subLayout->addWidget(PauseButton,0,0);

    subLayout->addWidget(filter->addToFilter,1,0);    
    subLayout->addWidget(filter->buttonAdd,1,1);
    subLayout->addWidget(filter->removeFromFilter,2,0);    
    subLayout->addWidget(filter->buttonRemove,2,1);
    layout->addLayout(subLayout,1,0);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.show();

    return app.exec();
}
