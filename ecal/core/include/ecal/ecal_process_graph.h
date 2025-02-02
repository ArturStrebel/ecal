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

namespace eCAL {
  namespace ProcessGraph {
    struct SProcessGraphEdge {
      bool isAlive;
      std::pair<int, int> edgeID;
      std::string publisherName;
      std::string subscriberName;
      std::string topicName;
      double bandwidth; // stored in Bit/s
    };

    struct SHostGraphEdge {
      bool isAlive;
      std::pair<int, int> edgeID;
      std::string outgoingHostName;
      std::string incomingHostName;
      double bandwidth; // stored in Bit/s
    };

    struct STopicTreeItem {
      bool isAlive;
      int processID;
      std::string topicName;
      std::string direction; // subscriber or publisher
      std::string processName;
      std::string description;
    };

    struct SProcessGraph {
      std::vector<SProcessGraphEdge> processEdges;
      std::vector<SHostGraphEdge> hostEdges;
      std::vector<STopicTreeItem> topicTreeItems;
    };

    ECAL_API SProcessGraph GetProcessGraph(const eCAL::Monitoring::SMonitoring &monitor);
  } // namespace ProcessGraph
} // namespace eCAL
