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
    std::pair<int,int> edgeID;

    // remove inactive processes from each graph
    for (auto it = m_process_graph.processEdges.begin(); it != m_process_graph.processEdges.end();)
    {
      if (!it->isAlive) 
        it = m_process_graph.processEdges.erase(it);
      else 
        (it++)->isAlive = false;
    }

    for (auto it = m_process_graph.hostEdges.begin(); it != m_process_graph.hostEdges.end();)
    {
      if (!it->isAlive)
        it = m_process_graph.hostEdges.erase(it);
      else 
      {
        it->bandwidth = 0; // recompute current bandwidth in every cycle
        (it++)->isAlive = false;
      }
    }

    for (auto it = m_process_graph.topicTreeItems.begin(); it != m_process_graph.topicTreeItems.end();)
    {
      if (!it->isAlive)
        it = m_process_graph.topicTreeItems.erase(it);
      else 
        (it++)->isAlive = false;
    }

    for( const auto pub : monitoring.publisher ) 
    {
      // create "empty" edge if no active subs for current pub
      int publisherConnections = pub.connections_loc + pub.connections_ext;
      int currentPublisherConnections = 0;
      if (publisherConnections == 0) 
      {
        edgeID = std::make_pair(pub.pid, 1000 * pub.pid ); // TODO: Find a truly unique solution?
        auto processEdge = FindProcessEdge(edgeID);
        if( processEdge == nullptr)
        {
          eCAL::Monitoring::STopicMon sub; // NOTE: Creating a temp sub here seems overkill.  
          sub.uname = "void";              // Maybe overload CreateProcessEdge(pub,edgeID)
          AddToProcessEdges(CreateProcessEdge(pub, sub, edgeID)); //TODO: What happens with host?
          m_process_graph.processEdges.insert(std::make_pair(edgeID, CreateProcessEdge(pub, sub, edgeID));
        }
        else           
          processEdge->isAlive = true;
      }

      // check all subscribers
      for( const auto sub : monitoring.subscriber )
      {
        // topic tree item for subscriber
        auto proc = FindProcess( sub.pid );
        if( proc == nullptr)
          AddToTopicTree(CreateTopicTreeItem(sub));
        else           
          proc->isAlive = true;

        // create "empty" edge if no active pubs for current sub
        int subscriberConnections = sub.connections_loc + sub.connections_ext;
        if (subscriberConnections == 0) 
        {
          edgeID = std::make_pair(sub.pid, 1000 * sub.pid ); // TODO: Find a truly unique solution?
          auto processEdge = FindProcessEdge(edgeID);
          if( processEdge == nullptr)
          {
            eCAL::Monitoring::STopicMon tmpPub; // NOTE: Creating a temp sub here seems overkill.  
            tmpPub.uname = "void";              // Maybe overload CreateProcessEdge(pub,edgeID)
            AddToProcessEdges(CreateProcessEdge(tmpPub, sub, edgeID)); //TODO: What happens with host?
          }
          else           
            processEdge->isAlive = true;
        }

        // things that have to happen for EVERY sub, must come before this point
        // after this, only pub/sub pairs in same topic are handled
        if( pub.tname != sub.tname ) continue;

        // process graph edge
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::ProcessGraph );
        auto processEdge = FindProcessEdge(edgeID);
        if( processEdge == nullptr)
          AddToProcessEdges( CreateProcessEdge( pub, sub, edgeID ) );
        else
          processEdge->isAlive = true;
        
        // host traffic edge
        edgeID = CreateEdgeID( pub, sub, eCAL::ProcessGraph::GraphType::HostTraffic );
        auto hostEdge = FindHostEdge(edgeID);
        if( hostEdge == nullptr )
          AddToHostEdges(CreateHostEdge(pub, sub, edgeID));
        else
        {
          hostEdge->isAlive = true;
          UpdateHostBandwidth(*hostEdge, GetBandwidth(pub));
        }

        // early exit out of sub loop if current pub found all its subs
        if (++currentPublisherConnections == publisherConnections) break;
      }

      // topic tree item for publisher
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

  double CProcessGraph::GetBandwidth(const eCAL::Monitoring::STopicMon& pub) 
  {
      return (pub.tsize * 8.0 * (pub.dfreq / 1000.0)) ; // Scale from (Byte * mHz) to (Bit / s)
  }

  std::pair<int,int> CProcessGraph::CreateEdgeID(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const int& graphType) 
  {
    if( graphType == eCAL::ProcessGraph::GraphType::HostTraffic ) 
      return std::pair<int,int>(pub.hid, sub.hid);
    return std::pair<int,int>(pub.pid, sub.pid);
  }

    void CProcessGraph::AddToProcessEdges(const eCAL::ProcessGraph::SProcessGraphEdge& newEdge) 
  {
    // This method assumes that edge is not already in the list
    m_process_graph.processEdges.push_back(newEdge);
  }

  eCAL::ProcessGraph::SProcessGraphEdge CProcessGraph::CreateProcessEdge(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub, const std::pair<int,int>& edgeID )
  {
    return 
    {
      true,
      edgeID,
      pub.uname, 
      sub.uname, 
      pub.tname,
      GetBandwidth(pub),
      nullptr,
      nullptr
      };
  }

  eCAL::ProcessGraph::SProcessGraphEdge* CProcessGraph::FindProcessEdge(const std::pair<int,int>& edgeID)
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

  eCAL::ProcessGraph::SHostGraphEdge CProcessGraph::CreateHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const std::pair<int,int>& edgeID)
  {
    return 
    {
      true,
      edgeID,
      pub.hname,
      sub.hname,
      GetBandwidth(pub)
    };
  }

  void CProcessGraph::UpdateHostBandwidth( eCAL::ProcessGraph::SHostGraphEdge& hostEdge, const double& bandwidthUpdate)
  {
    hostEdge.bandwidth += bandwidthUpdate;
  }

  eCAL::ProcessGraph::SHostGraphEdge* CProcessGraph::FindHostEdge( const std::pair<int,int>& edgeID )
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
      process.direction, // subscriber or publisher
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
