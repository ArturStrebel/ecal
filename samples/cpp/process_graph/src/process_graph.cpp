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

  eCAL::ProcessGraph::SProcessGraph processgraph;

  // monitor for ever
  while(eCAL::Ok())
  {
    // take snapshot :-)
    eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::All);

    processgraph = eCAL::ProcessGraph::GetProcessGraph(monitoring);

    // do stuff with process graph

    // sleep few milliseconds
    eCAL::Process::SleepMS(g_mon_timing);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
