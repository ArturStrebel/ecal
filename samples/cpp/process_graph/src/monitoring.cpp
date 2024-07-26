#include "monitoring.h"
#include <ecal/ecal.h>

Monitoring::Monitoring() {
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Monitoring::updateProcessGraph);
  updateProcessGraph();
  timer->start(500);
}

Monitoring::~Monitoring() { delete timer; }

void Monitoring::updateProcessGraph() {
  // processGraph.hostEdges = {
  //     {true, std::make_pair(1,2), "HPC 1", "EDGE 1", 2.34},
  //     {true, std::make_pair(2,1), "EDGE 1", "HPC 1", 50.0},
  //     {true, std::make_pair(1,1), "HPC 1", "HPC 1", 10.34},
  //     {true, std::make_pair(1,3), "HPC 1", "HPC Backup", 10.34},
  //     {true, std::make_pair(3,1), "HPC Backup", "HPC 1", 10.34}
  // };

  // processGraph.processEdges = {
  //     {true, std::make_pair(1,1), "ACU Process", "sub1", "Topic 2", 50.0},
  //     {true, std::make_pair(1,2), "ACU Process", "sub2", "Topic 3", 30.2},
  //     {true, std::make_pair(1,3), "ACU Process", "sub3", "Topic 4", 10.01},
  //     {true, std::make_pair(1,4), "ACU Process", "sub4", "Topic 5",
  //     12345678}, {true, std::make_pair(5,1), "Camera1", "ACU Process",
  //     "Rear_Camera", 90.8}, {true, std::make_pair(6,1), "Camera2", "ACU
  //     Process", "Rear_CameraHD", 11190.8}
  // };

  // processGraph.topicTreeItems = {
  //     {true, 1, "T1", "Publisher", "pub1", "Important"},
  //     {true, 2, "T1", "Publisher", "pub2", "stuff"},
  //     {true, 4, "T2", "Publisher", "pub1", "right"},
  //     {true, 5, "T3", "Publisher", "pub1", "here"}
  // };

  previousTopicTree = processGraph.topicTreeItems;
  eCAL::Monitoring::GetMonitoring(monitor, eCAL::Monitoring::Entity::All);
  processGraph = eCAL::ProcessGraph::GetProcessGraph(monitor);
  if (topicTreeHasChanged())
    emit updateTopicTree();
}

bool Monitoring::topicTreeHasChanged() {
  // TODO: Methode verschlanken.
  if (previousTopicTree.size() != processGraph.topicTreeItems.size())
    return true;

  bool found;
  for (const auto &it : previousTopicTree) {
    found = false;
    for (const auto &it2 : processGraph.topicTreeItems)
      if (it.topicID == it2.topicID)
        found = true;
    if (found == false)
      return true;
  }

  return false;
}

const eCAL::ProcessGraph::SProcessGraph &Monitoring::getProcessGraph() const {
  return processGraph;
}
