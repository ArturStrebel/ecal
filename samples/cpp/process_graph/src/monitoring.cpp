#include "monitoring.h"

Monitoring::Monitoring()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Monitoring::updateProcessGraph);
    timer->start(500);
}

Monitoring::~Monitoring()
{
    delete timer;
}

void Monitoring::updateProcessGraph()
{
    counter = counter + 1;

    if ((counter % 10) < 5) {
        process_graph.hostEdges = {
            {true, "TEST_EDGE_1", "HPC 1", "EDGE 1", 24.34},
            {true, "TEST_EDGE_2", "EDGE 1", "HPC 1", 5.0},
            {true, "TEST_EDGE_3", "HPC 1", "HPC 1", 1.34}
        };

        process_graph.processEdges = {
            {true, "process_sub1", "ACU Process", "sub1", "Topic 2", 5.0},
            {true, "process_sub2", "ACU Process", "sub2", "Topic 3", 3.2},
            {true, "process_sub3", "ACU Process", "sub3", "Topic 4", 0.01},
            {true, "process_sub4", "ACU Process", "sub4", "Topic 4", 0.01},
            {true, "publisher_process", "Camera", "ACU Process", "Rear_Camera", 9.8}
        };

        process_graph.topicTreeItems = {
            {true, 1, "T1", "Publisher", "pub1", "Important"},
            {true, 2, "T1", "Publisher", "pub2", "stuff"},
            {true, 3, "T1", "Subscriber", "sub1", "goes"},
            {true, 4, "T2", "Publisher", "pub1", "here"}
        };
    } else {
        process_graph.hostEdges = {
            {true, "TEST_EDGE_1", "HPC 1", "EDGE 1", 2.34},
            {true, "TEST_EDGE_2", "EDGE 1", "HPC 1", 50.0},
            {true, "TEST_EDGE_3", "HPC 1", "HPC 1", 10.34},
            {true, "TEST_EDGE_4", "HPC 1", "HPC Backup", 10.34},
            {true, "TEST_EDGE_5", "HPC Backup", "HPC 1", 10.34}
        };

        process_graph.processEdges = {
            {true, "process_sub1", "ACU Process", "sub1", "Topic 2", 50.0},
            {true, "process_sub2", "ACU Process", "sub2", "Topic 3", 30.2},
            {true, "process_sub3", "ACU Process", "sub3", "Topic 4", 10.01},
            {true, "publisher_process", "Camera", "ACU Process", "Rear_Camera", 90.8}
        };

        process_graph.topicTreeItems = {
            {true, 1, "T1", "Publisher", "pub1", "Important"},
            {true, 2, "T1", "Publisher", "pub2", "stuff"},
            {true, 4, "T2", "Publisher", "pub1", "here"},
            {true, 5, "T3", "Publisher", "pub1", "Yeeha"}
        };
    }

    
}

// Korrigierter Rückgabetyp
const eCAL::ProcessGraph::SProcessGraph& Monitoring::getProcessGraph() const
{
  return process_graph;
}


