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
      std::string CreateEdgeID(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub, const int& graphType);
      void UpdateProcessGraph(const eCAL::Monitoring::SMonitoring& monitoring);
      double GetBandwidth(const eCAL::Monitoring::STopicMon& pub);

      // Functions for process view
      eCAL::ProcessGraph::SProcessGraphEdge* FindProcessEdge(std::string edgeID);
      eCAL::ProcessGraph::SProcessGraphEdge CreateProcessEdge(const eCAL::Monitoring::STopicMon& pub , const eCAL::Monitoring::STopicMon& sub );
      void TryInsertProcessEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub );
      
      // Functions for host traffic view
      eCAL::ProcessGraph::SHostGraphEdge* FindHostEdge(std::string edgeID);
      eCAL::ProcessGraph::SHostGraphEdge CreateHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub );
      void TryInsertHostEdge(const eCAL::Monitoring::STopicMon& pub, const eCAL::Monitoring::STopicMon& sub );

      // Functions for topic tree
      eCAL::ProcessGraph::STopicTreeItem* FindTopicTreeItem(int topicID); 
      eCAL::ProcessGraph::STopicTreeItem CreateTopicTreeItem(const eCAL::Monitoring::STopicMon& process );
      void TryInsertTopicTreeItem(const eCAL::Monitoring::STopicMon& proc);

      eCAL::ProcessGraph::SProcessGraph m_process_graph; 
  };
}