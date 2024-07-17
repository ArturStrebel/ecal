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
    eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::All);
    process_graph = eCAL::ProcessGraph::GetProcessGraph(monitoring);
}

// Korrigierter Rückgabetyp
const eCAL::ProcessGraph::SProcessGraph& Monitoring::getProcessGraph() const
{
  return process_graph;
}


