// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "graphwidget.h"
#include "mainwindow.h"
#include "edge.h"
#include "node.h"
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

    QWidget *centralWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);


    // host graph
    QList<eCAL::ProcessGraph::SHostGraphEdge> host_edges = {
        {true, "TEST_EDGE_1", "HPC 1", "EDGE 1", 24.34},
        {true, "TEST_EDGE_1", "EDGE 1", "HPC 1", 5.0},
        {true, "TEST_EDGE_1", "HPC 1", "HPC 1", 1.34}};

    std::map<std::string, Node*> host_map;
    QList<Edge*> ui_edges;

    for (auto edge : host_edges) {
        bool outHostExists = host_map.find(edge.outgoingHostName) != host_map.end();
        bool inHostExists = host_map.find(edge.incomingHostName) != host_map.end();
        if (!inHostExists) {
            host_map[edge.incomingHostName] = new Node(Node::Host, QString::fromStdString(edge.incomingHostName));
        }
        if (!outHostExists) {
            host_map[edge.outgoingHostName] = new Node(Node::Host, QString::fromStdString(edge.outgoingHostName));
        }
        if (edge.outgoingHostName == edge.incomingHostName) {
            host_map[edge.outgoingHostName]->setInternalBandwidthMbits(edge.bandwidth);
        }
        ui_edges.append(new Edge(host_map[edge.outgoingHostName], host_map[edge.incomingHostName], true, false, "", edge.bandwidth));
    }
    
    QList<Node*> host_nodes;
    for (auto const& pair: host_map) {
        host_nodes.append(pair.second);
    }

    GraphWidget *widget1 = new GraphWidget(nullptr, host_nodes, ui_edges, "Host Network traffic");

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
