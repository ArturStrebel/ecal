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
    for( auto pub : monitoring.publisher ) 
    {
      for( auto sub : monitoring.subscriber )
      {
        if( pub.tname != sub.tname ) continue;

        // process graph
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::ProcessGraph );
        if( !IsContainedInList( edgeID, eCAL::ProcessGraph::GraphType::ProcessGraph) )
          AddToProcessEdgeList( CreateProcessEdge( pub, sub ) );

        // host traffic
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::HostTraffic );
        if( IsContainedInList( edgeID, eCAL::ProcessGraph::GraphType::HostTraffic) )
        {
          auto hostEdge = FindHostEdge(edgeID); // This should always return a valid edge due to above if
          UpdateHostBandwidth(hostEdge, 0.0 );
        } 
        else
        {
          AddToHostEdgeList(CreateHostEdge(pub, sub));
        }

        // topic tree
        // Check if topic is already in topic tree view
        // if yes, add pub and/or sub to tree
        // if no, add topic + pub/sub to tree
      }
    }
  }

  std::string CProcessGraph::CreateEdgeID(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::HostTraffic ) 
      return pub.hname + "_" + sub.hname;
    return std::to_string(pub.pid) + "_" + std::to_string(sub.pid);
  }

  bool CProcessGraph::IsContainedInList(const std::string& edgeID, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::ProcessGraph )
    {
      if( m_edgeHashTable.find(edgeID) == m_edgeHashTable.end())
        return false;
      return true;
    }

    if( graphType == eCAL::ProcessGraph::GraphType::HostTraffic )
    {
      auto hostEdge = FindHostEdge(edgeID); // returns an empty struct if edge not in list
      if ( hostEdge.edgeID == "" )
        return false;
      return true;
    }

    return false;
    
  }

  void CProcessGraph::AddToProcessEdgeList(const eCAL::ProcessGraph::SProcessGraphEdge& newEdge) 
  {
    // This method assumes that edge is not already in the list
    m_process_graph.processEdgeList.push_back(newEdge);
    m_edgeHashTable.insert( newEdge.edgeID );
  }

  eCAL::ProcessGraph::SProcessGraphEdge CProcessGraph::CreateProcessEdge(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub )
  {
    std::string edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::ProcessGraph );
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

  void CProcessGraph::AddToHostEdgeList(const eCAL::ProcessGraph::SHostGraphEdge& newHost )
  {
    m_process_graph.hostEdgeList.push_back(newHost);
  }

  eCAL::ProcessGraph::SHostGraphEdge CProcessGraph::CreateHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub)
  {
    std::string edgeID_ = CreateEdgeID(pub, sub, eCAL::ProcessGraph::GraphType::HostTraffic );
    return 
    {
      edgeID_,
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
    for( auto hostIt : m_process_graph.hostEdgeList)
      if( edgeID_ == hostIt.edgeID )
        return hostIt;
    return eCAL::ProcessGraph::SHostGraphEdge();
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
