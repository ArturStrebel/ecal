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

    eCAL::ProcessGraph::SProcessGraph process_graph;

    process_graph.hostEdges = {
        {true, "TEST_EDGE_1", "HPC 1", "EDGE 1", 24.34},
        {true, "TEST_EDGE_2", "EDGE 1", "HPC 1", 5.0},
        {true, "TEST_EDGE_3", "HPC 1", "HPC 1", 1.34}
    };

    process_graph.processEdges = {
        {true, "process_sub1", "ACU Process", "sub1", "Topic 2", 5.0, nullptr, nullptr},
        {true, "process_sub2", "ACU Process", "sub2", "Topic 3", 3.2, nullptr, nullptr},
        {true, "process_sub3", "ACU Process", "sub3", "Topic 4", 0.01, nullptr, nullptr},
        {true, "publisher_process", "Camera", "ACU Process", "Rear_Camera", 9.8, nullptr, nullptr}
    };

    QWidget *centralWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);

    std::map<std::string, Node*> host_map;
    QList<Edge*> host_edges;

    for (auto edge : process_graph.hostEdges) {
        // host_map.try_emplace(edge.outgoingHostName, new Node(Node::Host, QString::fromStdString(edge.outgoingHostName)));
        // host_map.try_emplace(edge.outgoingHostName, new Node(Node::Host, QString::fromStdString(edge.outgoingHostName)));
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
        host_edges.append(new Edge(host_map[edge.outgoingHostName], host_map[edge.incomingHostName], true, false, "", edge.bandwidth));
    }
    
    QList<Node*> host_nodes;
    for (auto const& pair: host_map) {
        host_nodes.append(pair.second);
    }

    GraphWidget *widget1 = new GraphWidget(nullptr, host_nodes, host_edges, "Host Network traffic");

    // Topic View
    MainWindow *widget2 = new MainWindow;

    std::map<std::string, Node*> process_map;
    QList<Edge*> process_edges;

    for (auto edge : process_graph.processEdges) {
        bool publisherExists = process_map.find(edge.publisherName) != process_map.end();
        bool subscriberExists = process_map.find(edge.subscriberName) != process_map.end();
        if (!publisherExists) {
            process_map[edge.publisherName] = new Node(Node::Process, QString::fromStdString(edge.publisherName));
        }
        if (!subscriberExists) {
            process_map[edge.subscriberName] = new Node(Node::Process, QString::fromStdString(edge.subscriberName));
        }
        process_edges.append(new Edge(process_map[edge.publisherName], process_map[edge.subscriberName], true, false, "", edge.bandwidth));
    }
    
    QList<Node*> process_nodes;
    for (auto const& pair: process_map) {
        process_nodes.append(pair.second);
    }

    // // Process Graph
    // Node *process = new Node(Node::Process, "ACU Process");
    // Node *sub1 = new Node(Node::Subscriber, "Sub1");
    // Node *sub2 = new Node(Node::Subscriber, "Sub2");
    // Node *sub3 = new Node(Node::Subscriber, "Sub3");
    // Node *publisher = new Node(Node::Publisher, "Camera");

    // QList<Node*> nodes = {process, sub1, sub2, sub3, publisher };

    // QList<Edge*> edges = {
    //     new Edge(process, sub1, true, false, "Topic2", 5.0),
    //     new Edge(process, sub2, true, false, "Topic 3", 3.2),
    //     new Edge(process, sub3, true, false, "Topic 4", 0.01),
    //     new Edge(publisher, process, true, false, "Rear_Camera", 9.8)};

    GraphWidget *widget3 = new GraphWidget(nullptr, process_nodes, process_edges, "Process Graph");

    layout->addWidget(widget1);
    layout->addWidget(widget2);
    layout->addWidget(widget3);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(centralWidget);

    mainWindow.show();
    return app.exec();
}
