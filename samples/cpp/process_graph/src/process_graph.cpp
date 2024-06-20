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

#include <ecal/ecal.h>

#include <iostream>

static int g_mon_timing = 1000;

int main(int argc, char **argv)
{
  // initialize eCAL core API
  eCAL::Initialize(argc, argv, "monitoring", eCAL::Init::All);

  // monitoring instance to store snapshot
  eCAL::Monitoring::SMonitoring monitoring;

  std::vector<eCAL::ProcessGraph::SProcessGraphEdge> edgeList;

  // create some test entries by hand
  edgeList.push_back({1, 2, 1.0, nullptr, nullptr});
  edgeList.push_back({1, 3, 5.5, nullptr, nullptr});
  edgeList.push_back({2, 3, 2.0, nullptr, nullptr});
  edgeList.push_back({3, 4, 0.1, nullptr, nullptr});
  edgeList[0].publisherNext = &edgeList[1];
  edgeList[1].subscriberNext = &edgeList[2];
  // graph should looks something like this:
  //  (1) -> (2)
  //      \   |
  //       v  v 
  //  (4)<-(3)

  // monitor for ever
  while(eCAL::Ok())
  {
    // take snapshot :-)
    eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::All);

    edgeList = eCAL::ProcessGraph::GetEdgeList(monitoring);

    // do stuff with edge list

    // sleep few milliseconds
    eCAL::Process::SleepMS(g_mon_timing);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
