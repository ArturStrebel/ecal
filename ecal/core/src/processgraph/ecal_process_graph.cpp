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
#include <random>

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

  void CProcessGraph::Create()
  {

  }

  void CProcessGraph::Destroy()
  {

  }

  eCAL::ProcessGraph::SProcessGraph CProcessGraph::GetProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring) 
  {
    UpdateProcessGraph(monitoring);
    return m_process_graph; 
  }

  void CProcessGraph::UpdateProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring)
  {
    std::string edgeID;
    long long publisherBandwidth;

    // remove inactive processes from each graph
    for (auto it = m_process_graph.processEdges.begin(); it != m_process_graph.processEdges.end();)
    {
      if (!it->isAlive) 
      {
        m_edgeHashTable.erase(it->edgeID);
        it = m_process_graph.processEdges.erase(it);
      }
       
      else 
      {
        it->isAlive = false;
        ++it;
      }
    }

    for (auto it = m_process_graph.hostEdges.begin(); it != m_process_graph.hostEdges.end();)
    {
      if (!it->isAlive)

        it = m_process_graph.hostEdges.erase(it);
      else 
      {
        it->bandwidth = 0; // recompute current bandwidth in every cycle
        it->isAlive = false;
        ++it;
      }
    }

    for (auto it = m_process_graph.topicTreeItems.begin(); it != m_process_graph.topicTreeItems.end();)
    {
      if (!it->isAlive)
        it = m_process_graph.topicTreeItems.erase(it);
      else 
      {
        it->isAlive = false;
        ++it;
      }
    }

    for( const auto pub : monitoring.publisher ) 
    {

      publisherBandwidth = GetBandwidth(pub.pid, monitoring.process);

      for( const auto sub : monitoring.subscriber )
      {
        // topic tree for subscriber
        auto proc = FindProcess( sub.pid );
        if( proc == nullptr)
          AddToTopicTree(CreateTopicTreeItem(sub));
        else           
          proc->isAlive = true;

        if( pub.tname != sub.tname ) continue;

        // process graph
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::ProcessGraph );
        auto processEdge = FindProcessEdge(edgeID);
        if( processEdge == nullptr)
          AddToProcessEdges( CreateProcessEdge( pub, sub, edgeID, publisherBandwidth ) );
        else
          processEdge->isAlive = true;
        
        // host traffic
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::HostTraffic );
        auto hostEdge = FindHostEdge(edgeID);
        if( hostEdge == nullptr )
          AddToHostEdges(CreateHostEdge(pub, sub, edgeID, publisherBandwidth));
        else
        {
          hostEdge->isAlive = true;
          UpdateHostBandwidth(*hostEdge, publisherBandwidth);
        }
      }

      // topic tree for publisher
      auto proc = FindProcess( pub.pid );
      if( proc == nullptr)
        AddToTopicTree(CreateTopicTreeItem(pub));
      else 
        proc->isAlive = true;    
    }

    // sort topic tree, first w.r.t. topic name, second process name
    std::sort(m_process_graph.topicTreeItems.begin(), m_process_graph.topicTreeItems.end(),
               [] (const eCAL::ProcessGraph::STopicTreeItem& lhs, const eCAL::ProcessGraph::STopicTreeItem& rhs) 
               {
                return (lhs.topicName == rhs.topicName ? (lhs.direction < rhs.direction) : (lhs.topicName < rhs.topicName) );
               }
      );
  }

  double CProcessGraph::GetBandwidth(const int& processID, const std::vector<eCAL::Monitoring::SProcessMon> processList) 
  {
    auto it = std::find_if(processList.begin(), processList.end(), 
      [processID] ( const eCAL::Monitoring::SProcessMon& it) 
      {
       return it.pid == processID;
      });
        
    if ( it != processList.end() )
      return it->datawrite * 1024 * 1024 / 8; // Scale from Byte/s to Mbit/s
    return 0;
  }

  std::string CProcessGraph::CreateEdgeID(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::HostTraffic ) 
      return pub.hname + "_" + sub.hname;
    return std::to_string(pub.pid) + "_" + std::to_string(sub.pid);
  }

    void CProcessGraph::AddToProcessEdges(const eCAL::ProcessGraph::SProcessGraphEdge& newEdge) 
  {
    // This method assumes that edge is not already in the list
    m_process_graph.processEdges.push_back(newEdge);
    m_edgeHashTable.insert( newEdge.edgeID );
  }

  eCAL::ProcessGraph::SProcessGraphEdge CProcessGraph::CreateProcessEdge(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub, const std::string& edgeID, const  double& hostedge )
  {
    return 
    {
      true,
      edgeID,
      pub.uname, 
      sub.uname, 
      pub.tname,
      hostedge,
      nullptr,
      nullptr
      };
  }

  eCAL::ProcessGraph::SProcessGraphEdge* CProcessGraph::FindProcessEdge(const std::string& edgeID)
  {
    auto it = std::find_if(m_process_graph.processEdges.begin(), m_process_graph.processEdges.end(), 
      [edgeID] ( const eCAL::ProcessGraph::SProcessGraphEdge& it) 
      {
       return it.edgeID == edgeID;
      });
    
    if (it != m_process_graph.processEdges.end())
      return &*it;
    return nullptr;
  }

  void CProcessGraph::AddToHostEdges (const eCAL::ProcessGraph::SHostGraphEdge& newHost )
  {
    m_process_graph.hostEdges.push_back(newHost);
  }

  eCAL::ProcessGraph::SHostGraphEdge CProcessGraph::CreateHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const std::string& edgeID, const double& bandwidth)
  {
    return 
    {
      true,
      edgeID,
      pub.hname,
      sub.hname,
      bandwidth
    };
  }

  void CProcessGraph::UpdateHostBandwidth( eCAL::ProcessGraph::SHostGraphEdge& hostEdge, const double& bandwidthUpdate)
  {
    hostEdge.bandwidth += bandwidthUpdate;
  }

  eCAL::ProcessGraph::SHostGraphEdge* CProcessGraph::FindHostEdge( const std::string& edgeID )
  {
    auto it = std::find_if(m_process_graph.hostEdges.begin(), m_process_graph.hostEdges.end(), 
      [edgeID] ( const eCAL::ProcessGraph::SHostGraphEdge& it) 
      {
       return it.edgeID == edgeID;
      });
    
    if (it != m_process_graph.hostEdges.end())
      return &*it;
    return nullptr;
  }

  eCAL::ProcessGraph::STopicTreeItem* CProcessGraph::FindProcess( const int& processID )
  {
    auto it = std::find_if(m_process_graph.topicTreeItems.begin(), m_process_graph.topicTreeItems.end(), 
      [processID] ( const eCAL::ProcessGraph::STopicTreeItem& it) 
      {
       return it.processID == processID;
      });
        
    if ( it != m_process_graph.topicTreeItems.end() )
      return &*it;
    return nullptr;
  }

  void CProcessGraph::AddToTopicTree (const eCAL::ProcessGraph::STopicTreeItem& newProcess )
  {
    m_process_graph.topicTreeItems.push_back(newProcess);
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
