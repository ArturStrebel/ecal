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

  void CProcessGraph::UpdateProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring)
  {
    std::string edgeID;
    for( const auto pub : monitoring.publisher ) 
    {
      for( const auto sub : monitoring.subscriber )
      {
        // topic tree for subscriber
        if( !IsContainedIn(sub.pid, eCAL::ProcessGraph::GraphType::TopicTree))
        {
          AddToTopicTree(CreateTopicTreeItem(sub));
        }

        if( pub.tname != sub.tname ) continue;

        // process graph
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::ProcessGraph );
        if( !IsContainedIn( edgeID, eCAL::ProcessGraph::GraphType::ProcessGraph) )
          AddToProcessEdges( CreateProcessEdge( pub, sub, edgeID ) );

        // host traffic
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::HostTraffic );
        if( IsContainedIn( edgeID, eCAL::ProcessGraph::GraphType::HostTraffic) )
        {
          auto hostEdge = FindHostEdge(edgeID); // This should always return a valid edge due to above if
          UpdateHostBandwidth(hostEdge, 0.0 );
        } 
        else
        {
          AddToHostEdges(CreateHostEdge(pub, sub, edgeID));
        }
      }
      // topic tree for publisher
        if( !IsContainedIn(pub.pid, eCAL::ProcessGraph::GraphType::TopicTree))
        {
          AddToTopicTree(CreateTopicTreeItem(pub));
        }
    }
  }

  std::string CProcessGraph::CreateEdgeID(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::HostTraffic ) 
      return pub.hname + "_" + sub.hname;
    return std::to_string(pub.pid) + "_" + std::to_string(sub.pid);
  }

  
  bool CProcessGraph::IsContainedIn(const std::string& edgeID, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::ProcessGraph )
    {
      if( m_edgeHashTable.find(edgeID) == m_edgeHashTable.end())
        return false;
      return true;
    }

    if( graphType == eCAL::ProcessGraph::GraphType::HostTraffic )
    {
      const auto hostEdge = FindHostEdge(edgeID); // returns an empty struct if edge not in list
      if ( hostEdge.edgeID == "" )
        return false;
      return true;
    }
    return false;
  }

  bool CProcessGraph::IsContainedIn(const int& processID, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::TopicTree )
    {
      const auto process = FindProcess(processID); // returns an empty struct if edge not in list
      if ( process.processID == 0 )
        return false;
      return true;
    }
    return false;
  }

  void CProcessGraph::AddToProcessEdges(const eCAL::ProcessGraph::SProcessGraphEdge& newEdge) 
  {
    // This method assumes that edge is not already in the list
    m_process_graph.processEdges.push_back(newEdge);
    m_edgeHashTable.insert( newEdge.edgeID );
  }

  eCAL::ProcessGraph::SProcessGraphEdge CProcessGraph::CreateProcessEdge(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub, std::string edgeID )
  {
    return 
    {
      edgeID,
      pub.uname, 
      sub.uname, 
      pub.tname,
      0.0, //TODO: How to get bandwidth? 
      nullptr,
      nullptr
      };
  }

  eCAL::ProcessGraph::SProcessGraph CProcessGraph::GetProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring) 
  {
    UpdateProcessGraph(monitoring);
    return m_process_graph; 
  }

  void CProcessGraph::AddToHostEdges (const eCAL::ProcessGraph::SHostGraphEdge& newHost )
  {
    m_process_graph.hostEdges.push_back(newHost);
  }

  eCAL::ProcessGraph::SHostGraphEdge CProcessGraph::CreateHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, std::string edgeID)
  {
    return 
    {
      edgeID,
      pub.hname,
      sub.hname,
      0.0
    };
  }

  void CProcessGraph::UpdateHostBandwidth( eCAL::ProcessGraph::SHostGraphEdge& hostEdge, double bandwidthUpdate)
  {
    hostEdge.bandwidth += bandwidthUpdate;
  }

  eCAL::ProcessGraph::SHostGraphEdge CProcessGraph::FindHostEdge( const std::string& edgeID_ )
  {
    for( auto hostIt : m_process_graph.hostEdges )
      if( edgeID_ == hostIt.edgeID )
        return hostIt;
    return eCAL::ProcessGraph::SHostGraphEdge();
  }

  eCAL::ProcessGraph::STopicTreeItem CProcessGraph::FindProcess( const int& processID_ )
  {
    for( auto processIt : m_process_graph.topicTreeItems )
      if( processID_ == processIt.processID )
        return processIt;
    return eCAL::ProcessGraph::STopicTreeItem();
  }

  void CProcessGraph::AddToTopicTree (const eCAL::ProcessGraph::STopicTreeItem& newProcess )
  {
    m_process_graph.topicTreeItems.push_back(newProcess);
  }

  eCAL::ProcessGraph::STopicTreeItem CProcessGraph::CreateTopicTreeItem(const eCAL::Monitoring::STopicMon& process )
  {
    return 
    {
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
