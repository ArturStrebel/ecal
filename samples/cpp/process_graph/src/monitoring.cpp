#include "monitoring.h"
#include <ecal/ecal.h>

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
    process_graph.hostEdges = 
    {
        {true, std::make_pair(1,1), "Laptop", "Laptop", 10000},
        {true, std::make_pair(1,1), "Laptop", "Laptop", 100},
        {true, std::make_pair(1,2), "Laptop", "Handy", 10},
        {true, std::make_pair(3,2), "PC", "Handy", 50.3},
        {true, std::make_pair(3,1), "PC", "Laptop", 123456789},
        {true, std::make_pair(2,1), "Handy", "Laptop", 0}
    };
 
    eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::All);
    // process_graph = eCAL::ProcessGraph::GetProcessGraph(monitoring);
}

const eCAL::ProcessGraph::SProcessGraph& Monitoring::getProcessGraph() const
{
  return process_graph;
}


