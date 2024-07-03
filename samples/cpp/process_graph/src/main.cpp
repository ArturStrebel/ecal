// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphwidget.h"
#include "mainwindow.h"
#include "node.h"
#include "edge.h"


#include <QApplication>
#include <QTime>
#include <QMainWindow>
#include <QHBoxLayout>


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget *centralWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);


    // host graph
    Node *host1 = new Node(Node::Host, "HPC 1", 0.0);
    Node *host2 = new Node(Node::Host, "EDGE 1", 1.3);
    Node *host3 = new Node(Node::Host, "ZONE REAR", 100.7);
    Node *host4 = new Node(Node::Host, "EDG 2", 32.8);

    QList<Node*> host_nodes = {host1, host2, host3, host4};

    QList<Edge*> host_edges = {
        new Edge(host1, host2, false, true, "", 5.0),
        new Edge(host2, host1, false, true, "", 5.0),
        new Edge(host1, host3, false, true, "", 3.2),
        new Edge(host3, host1, false, true, "", 2.3),
        new Edge(host1, host4, false, true, "", 0.01),
        new Edge(host4, host1, false, true, "", 0.9),
        new Edge(host3, host4, false, true, "", 9.8),
        new Edge(host4, host3, false, true, "", 0.1),
        new Edge(host2, host3, false, true, "", 9.8),
        new Edge(host3, host2, false, true, "", 6.5)};

    GraphWidget *widget1 = new GraphWidget(nullptr, host_nodes, host_edges, "Host Network traffic");

    // Topic View
    MainWindow *widget2 = new MainWindow;

    // Process Graph
    Node *process = new Node(Node::Process, "ACU Process");
    Node *sub1 = new Node(Node::Subscriber, "Sub1");
    Node *sub2 = new Node(Node::Subscriber, "Sub2");
    Node *sub3 = new Node(Node::Subscriber, "Sub3");
    Node *publisher = new Node(Node::Publisher, "Camera");

    QList<Node*> nodes = {process, sub1, sub2, sub3, publisher};

    QList<Edge*> edges = {
        new Edge(process, sub1, true, false, "Topic2", 5.0),
        new Edge(process, sub2, true, false, "Topic 3", 3.2),
        new Edge(process, sub3, true, false, "Topic 4", 0.01),
        new Edge(publisher, process, true, false, "Rear_Camera", 9.8)};

    GraphWidget *widget3 = new GraphWidget(nullptr, nodes, edges, "Process Graph");

    layout->addWidget(widget1);
    layout->addWidget(widget2);
    layout->addWidget(widget3);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.show();
    return app.exec();
}
