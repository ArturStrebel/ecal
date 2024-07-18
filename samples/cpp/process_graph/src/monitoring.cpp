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
    previousTopicTree = process_graph.topicTreeItems;
    eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::All);
    process_graph = eCAL::ProcessGraph::GetProcessGraph(monitoring);
    if(topicTreeHasChanged())
        emit updateTopicTree();        
}

bool Monitoring::topicTreeHasChanged()
{
    if (previousTopicTree.size() != process_graph.topicTreeItems.size()) return true;

    bool found;
    for (const auto& it : previousTopicTree) 
    {
        found = false;
        for (const auto& it2 : process_graph.topicTreeItems)
            if (it.topicID == it2.topicID) found = true;
        if (found == false) return true;
    }
        
    return false;
}

const eCAL::ProcessGraph::SProcessGraph& Monitoring::getProcessGraph() const
{
  return process_graph;
}


