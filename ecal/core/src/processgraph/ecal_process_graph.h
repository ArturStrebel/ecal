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

  class CProcessGraphDCEL
  {
    public:

      CProcessGraphDCEL() = default;
      ~CProcessGraphDCEL() = default;

      void Create();
      void Destroy();

      std::vector<eCAL::ProcessGraph::SProcessGraphEdge> GetEdgeList(const eCAL::Monitoring::SMonitoring& monitoring);

    private:

      void UpdateEdgeList(const eCAL::Monitoring::SMonitoring&);
      std::string CreateEdgeID(const int& pubID, const int& subID);
      bool IsContainedInList(const std::string& edgeID);
      void AddToEdgeList(const eCAL::ProcessGraph::SProcessGraphEdge&);

      std::unordered_set<std::string> m_edgeHashTable; 
      std::vector<eCAL::ProcessGraph::SProcessGraphEdge> m_edgeList;
  };
}