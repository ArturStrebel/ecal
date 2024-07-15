/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include "ecal_process_graph.h"
#include "ecal_global_accessors.h"
#include <algorithm>

namespace eCAL
{

  CProcessGraph::CProcessGraph() 
  {
    Create(); 
  }

  CProcessGraph::~CProcessGraph() 
  {
    Destroy(); 
  }

  void CProcessGraph::Create(){}

  void CProcessGraph::Destroy(){}

  eCAL::ProcessGraph::SProcessGraph CProcessGraph::GetProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring) 
  {
    UpdateProcessGraph(monitoring);
    return m_process_graph; 
  }

  eCAL::ProcessGraph::SProcessGraphEdge* CProcessGraph::FindProcessEdge(std::string edgeID) 
  {
    for (auto it = m_process_graph.processEdges.begin(); it != m_process_graph.processEdges.end(); ++it) 
      if(it->edgeID == edgeID)
        return &*it;
    return nullptr;
  }

  eCAL::ProcessGraph::SHostGraphEdge* CProcessGraph::FindHostEdge(std::string edgeID) 
  {
    for (auto it = m_process_graph.hostEdges.begin(); it != m_process_graph.hostEdges.end(); ++it) 
      if(it->edgeID == edgeID)
        return &*it;
    return nullptr;
  }

  eCAL::ProcessGraph::STopicTreeItem* CProcessGraph::FindTopicTreeItem(int topicID) 
  {
    for (auto it = m_process_graph.topicTreeItems.begin(); it != m_process_graph.topicTreeItems.end(); ++it) 
      if(it->topicID == topicID)
        return &*it;
    return nullptr;
  }

  void CProcessGraph::TryInsertProcessEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub ) 
  {
    auto edgeID = std::to_string(pub.pid) + "_" + std::to_string(sub.pid);
    auto processPtr = FindProcessEdge(edgeID);
    if( processPtr == nullptr)
      m_process_graph.processEdges.push_back(CreateProcessEdge(pub, sub));
    else
      processPtr->isAlive = true;
  }

  void CProcessGraph::TryInsertHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub ) 
  {
    auto edgeID = std::to_string(pub.hid) + "_" + std::to_string(sub.hid);
    auto processPtr = FindHostEdge(edgeID);
    if( processPtr == nullptr)
      m_process_graph.hostEdges.push_back(CreateHostEdge(pub, sub));
    else 
    {
      processPtr->isAlive = true;    
      processPtr->bandwidth += GetBandwidth(pub);
    }      
  }

  void CProcessGraph::TryInsertTopicTreeItem(const eCAL::Monitoring::STopicMon& proc)
  {
    auto processPtr = FindTopicTreeItem(proc.pid);
    if( processPtr == nullptr)
      m_process_graph.topicTreeItems.push_back(CreateTopicTreeItem(proc));
    else 
      processPtr->isAlive = true;
  }

  void CProcessGraph::UpdateProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring)
  {
    // remove inactive processes from each graph
    for (auto it = m_process_graph.processEdges.begin(); it != m_process_graph.processEdges.end();)
    {
      if (!it->isAlive) 
        it = m_process_graph.processEdges.erase(it);
      else 
        it++->isAlive = false;
    }

    for (auto it = m_process_graph.hostEdges.begin(); it != m_process_graph.hostEdges.end();)
    {
      if (!it->isAlive)
        it = m_process_graph.hostEdges.erase(it);
      else 
      {
        it->bandwidth = 0; // recompute current bandwidth in every cycle
        it++->isAlive = false;
      }
    }

    for (auto it = m_process_graph.topicTreeItems.begin(); it != m_process_graph.topicTreeItems.end();)
    {
      if (!it->isAlive)
        it = m_process_graph.topicTreeItems.erase(it);
      else 
        it++->isAlive = false;
    }

    for( const auto pub : monitoring.publisher ) 
    {
      int publisherConnections = pub.connections_loc + pub.connections_ext;
      int currentPublisherConnections = 0;

      // create "empty" edge if no active subs for current pub
      if (publisherConnections == 0) 
      {
        eCAL::Monitoring::STopicMon sub;
        sub.uname = "void";
        sub.pid = -pub.pid;
        TryInsertProcessEdge(pub, sub);
      }

      // check all subscribers
      for( const auto sub : monitoring.subscriber )
      {
        int subscriberConnections = sub.connections_loc + sub.connections_ext;

        // create "empty" edge if no active pubs for current sub
        if (subscriberConnections == 0) 
        {
          eCAL::Monitoring::STopicMon tmpPub;
          tmpPub.uname = "void";
          tmpPub.pid = -sub.pid;
          TryInsertProcessEdge(tmpPub, sub);
        }

        // topic tree for subscriber
        TryInsertTopicTreeItem(sub);

        if( pub.tname != sub.tname ) continue;

        // process graph
        TryInsertProcessEdge(pub, sub);
        
        // host traffic
        TryInsertHostEdge(pub, sub);

        if (++currentPublisherConnections == publisherConnections) break;
      }

      // topic tree for publisher
      TryInsertTopicTreeItem(pub);
    }

    // sort topic tree, first w.r.t. topic name, second process name
    std::sort(m_process_graph.topicTreeItems.begin(), m_process_graph.topicTreeItems.end(),
               [] (const eCAL::ProcessGraph::STopicTreeItem& lhs, const eCAL::ProcessGraph::STopicTreeItem& rhs) 
               {
                return (lhs.topicName == rhs.topicName ? (lhs.direction < rhs.direction) : (lhs.topicName < rhs.topicName) );
               }
      );
  }

  double CProcessGraph::GetBandwidth(const eCAL::Monitoring::STopicMon& pub) 
  {
      return (pub.tsize * 8.0 * (pub.dfreq / 1000.0)) ; // Scale from (Byte * mHz) to (Bit / s)
  }

  std::string CProcessGraph::CreateEdgeID(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::HostTraffic ) 
      return pub.hname + "_" + sub.hname;
    return std::to_string(pub.pid) + "_" + std::to_string(sub.pid);
  }

  eCAL::ProcessGraph::SProcessGraphEdge CProcessGraph::CreateProcessEdge(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub )
  {
    return 
    {
      true,
      std::to_string(pub.pid) + "_" + std::to_string(sub.pid),
      pub.uname, 
      sub.uname, 
      pub.tname,
      GetBandwidth(pub),
      };
  }

  eCAL::ProcessGraph::SHostGraphEdge CProcessGraph::CreateHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub)
  {
    return 
    {
      true,
      std::to_string(pub.hid) + "_" + std::to_string(sub.hid),
      pub.hname,
      sub.hname,
      GetBandwidth(pub)
    };
  }

  eCAL::ProcessGraph::STopicTreeItem CProcessGraph::CreateTopicTreeItem(const eCAL::Monitoring::STopicMon& process )
  {
    return 
    {
      true,
      process.pid,
      process.tname,
      process.direction, //Subscriber or Publisher
      process.uname,
      "test"
    };
  }

  namespace ProcessGraph
  {
    SProcessGraph GetProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring)
    {
      if (g_processgraph() != nullptr)
      {
        return g_processgraph()->GetProcessGraph(monitoring);
      }
      return(SProcessGraph());
    }
  }
}
