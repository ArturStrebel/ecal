#include "monitoring.h"

Monitoring::Monitoring()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Monitoring::updateProcessGraph);
    updateProcessGraph();
    timer->start(500);
}

Monitoring::~Monitoring()
{
    delete timer;
}

void Monitoring::updateProcessGraph()
{
    process_graph.hostEdges = {
        {true, std::make_pair(1,1), "HPC 1", "EDGE 1", 24.34},
        {true, std::make_pair(2,1), "EDGE 1", "HPC 1", 5.0},
        {true, std::make_pair(3,1), "HPC 1", "HPC 1", 1.34}
    };

    process_graph.processEdges = {
        {true, std::make_pair(1,2), "ACU Process", "sub1", "Topic 2", 5.0},
        {true, std::make_pair(2,2), "ACU Process", "sub2", "Topic 3", 3.2},
        {true, std::make_pair(3,2), "ACU Process", "sub3", "Topic 4", 0.01},
        {true, std::make_pair(4,2), "ACU Process", "sub4", "Topic 4", 0.01},
        {true, std::make_pair(5,2), "Camera", "ACU Process", "Rear_Camera", 9.8}
    };

    process_graph.topicTreeItems = {
        {true, 1, "T1", "Publisher", "pub1", "Important"},
        {true, 2, "T1", "Publisher", "pub2", "stuff"},
        {true, 3, "T1", "Subscriber", "sub1", "goes"},
        {true, 4, "T2", "Publisher", "pub1", "here"}
    };
}

// Korrigierter Rückgabetyp
const eCAL::ProcessGraph::SProcessGraph& Monitoring::getProcessGraph() const
{
  return process_graph;
}


