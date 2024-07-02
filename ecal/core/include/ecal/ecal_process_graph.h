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

#pragma once

#include <vector>
#include <ecal/ecal_os.h>

namespace eCAL
{
  namespace ProcessGraph
  {
    namespace GraphType
    {
      constexpr unsigned int ProcessGraph  = 0x001;
      constexpr unsigned int HostTraffic   = 0x002;
      constexpr unsigned int TopicTree     = 0x004;
    }

    struct SProcessGraphEdge
    {
      std::string edgeID;
      std::string publisherName;
      std::string subscriberName;
      std::string topicName;
      double bandwidth;

      SProcessGraphEdge* publisherNext = nullptr;
      SProcessGraphEdge* subscriberNext = nullptr;
    };

    struct SHostGraphEdge
    {
      std::string edgeID;
      std::string outgoingHostName;
      std::string incomingHostName;
      double bandwidth;
    };

    struct SProcessGraph
    {
      std::vector<SProcessGraphEdge> processEdgeList;
      std::vector<SHostGraphEdge> hostEdgeList;
      // TODO: Topic tree
    };

    ECAL_API SProcessGraph GetProcessGraph(const eCAL::Monitoring::SMonitoring&);
    
  }
}
