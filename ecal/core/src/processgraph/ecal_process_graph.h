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

/**
 * @file   ecal_process_graph.h
 * @brief  eCAL process graph class
**/

#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <ecal/types/monitoring.h>
#include <ecal/ecal.h>


namespace eCAL
{

  class CProcessGraph
  {
    public:

      CProcessGraph();
      ~CProcessGraph();

      void Create();
      void Destroy();

      eCAL::ProcessGraph::SProcessGraph GetProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring);

    private:

      // General functions
      std::string CreateEdgeID(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub, const int& graphType);
      void UpdateProcessGraph(const eCAL::Monitoring::SMonitoring&);

      // Functions for process view
      void AddToProcessEdges(const eCAL::ProcessGraph::SProcessGraphEdge&);
      eCAL::ProcessGraph::SProcessGraphEdge CreateProcessEdge(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub, std::string edgeID );
      eCAL::ProcessGraph::SProcessGraphEdge* FindProcessEdge(const std::string& edgeID);
      
      // Functions for host traffic view
      void AddToHostEdges(const eCAL::ProcessGraph::SHostGraphEdge& newHost);
      eCAL::ProcessGraph::SHostGraphEdge CreateHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, std::string edgeID);
      void UpdateHostBandwidth(eCAL::ProcessGraph::SHostGraphEdge& hostEdge, double bandwidthUpdate);
      eCAL::ProcessGraph::SHostGraphEdge* FindHostEdge( const std::string& hostID );

      // Functions for topic tree
      void CProcessGraph::AddToTopicTree (const eCAL::ProcessGraph::STopicTreeItem& newProcess );
      eCAL::ProcessGraph::STopicTreeItem CProcessGraph::CreateTopicTreeItem(const eCAL::Monitoring::STopicMon& process );
      eCAL::ProcessGraph::STopicTreeItem* FindProcess( const int& processID );

      std::unordered_set<std::string> m_edgeHashTable; 
      eCAL::ProcessGraph::SProcessGraph m_process_graph; 
  };
}