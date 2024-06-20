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

  void CProcessGraphDCEL::Create()
  {

  }

  void CProcessGraphDCEL::Destroy()
  {

  }

  void CProcessGraphDCEL::UpdateEdgeList(const eCAL::Monitoring::SMonitoring& monitoring)
  {
    std::string edgeID;
    for(auto pub : monitoring.publisher) 
    {
      for(auto sub : monitoring.subscriber)
      {
        if( sub.tname != pub.tname ) continue;

        edgeID = CProcessGraphDCEL::CreateEdgeID(pub.pid, sub.pid);
        if( IsContainedInList(edgeID) ) continue;

        std::cout << "Found an edge" << std::endl;
      }
    }
  }

  std::string CProcessGraphDCEL::CreateEdgeID(const int& pubID, const int& subID) 
  {
    return std::to_string(pubID) + "_" + std::to_string(subID);
  }

  bool CProcessGraphDCEL::IsContainedInList(const std::string& edgeID) 
  {
    if( edgeID == "123" )
      return true;
    return false;
  }

  void CProcessGraphDCEL::AddToEdgeList(const eCAL::ProcessGraph::SProcessGraphEdge&) 
  {

  }

  std::vector<eCAL::ProcessGraph::SProcessGraphEdge> CProcessGraphDCEL::GetEdgeList(const eCAL::Monitoring::SMonitoring& monitoring) 
  {
    UpdateEdgeList(monitoring);
    return m_edgeList;
  }

  namespace ProcessGraph
  {
    std::vector<SProcessGraphEdge> GetEdgeList(const eCAL::Monitoring::SMonitoring& monitoring)
    {
      if (g_processgraph_dcel() != nullptr)
      {
        return g_processgraph_dcel()->GetEdgeList(monitoring);
      }
      return(std::vector<SProcessGraphEdge>());
    }
  }
}
