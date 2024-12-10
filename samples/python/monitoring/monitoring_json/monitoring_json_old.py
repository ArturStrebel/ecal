# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================
import os
import sys
import time
import ecal.core.core as ecal_core

from DatabaseHandler import DatabaseHandler
from callbacks import host_monitor_callback
from helpers import convert_bytes_to_str, create_host_graph_list_from_topics_and_hosts, export_graph_structure

from ecal.core.subscriber import ProtoSubscriber
import proto_messages.mma_pb2 as mma_pb2
from google.protobuf.json_format import MessageToDict


def host_monitor_callback(topic_name, msg, time):
  db_handler_callback = DatabaseHandler()

  pids = db_handler_callback.execute_command('SELECT DISTINCT PID from current_processes').fetchall() # currently active processes
  pids = {id[0] for id in pids}
    
  host = MessageToDict(message=msg)
  hname = topic_name.split('_')[2]
  db_handler_callback.insert_host(hname, host, 'hosts')
  db_handler_callback.execute_command_with_args('DELETE FROM current_hosts WHERE hname = ?', (hname,))
  db_handler_callback.insert_host(hname, host, 'current_hosts')
  db_handler_callback.commit()

  processes = host['process']
  processes = [p for p in processes if p.get('id', -1) in pids]
  for process in processes:
    db_handler_callback.insert_process_performance(hname, process, 'process_performance')

  db_handler_callback.commit()
  db_handler_callback.close()

DB_CONNECTION = os.path.dirname(os.path.abspath(__file__)) + "\\ecal_monitoring.db"


def main():
    # print eCAL version and date
    print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))

    # initialize eCAL API
    ecal_core.initialize(sys.argv, "monitoring")

    # initialize eCAL monitoring API
    ecal_core.mon_initialize()
    time.sleep(2)

    db_handler = DatabaseHandler(db_string=DB_CONNECTION)

    host_subscribers = dict()

    # Create tables
    db_handler.create_process_table("processes")
    db_handler.create_process_table("current_processes")
    db_handler.create_process_table("previous_processes")
    db_handler.create_process_table("dropped_processes")

    db_handler.create_service_table("services")
    db_handler.create_service_table("current_services")
    db_handler.create_service_table("previous_services")

    db_handler.create_topic_table("topics")
    db_handler.create_topic_table("current_topics")
    db_handler.create_topic_table("previous_topics")

    db_handler.create_host_table("hosts")
    db_handler.create_host_table("current_hosts")

    db_handler.create_process_performance_table("process_performance")

    # host graph tables (dont change any names in these two tables!)
    db_handler.create_edge_table("edges")
    db_handler.create_node_table("nodes")

    db_handler.commit()

    
    
    # print eCAL entities
    while ecal_core.ok():
        # convert 'bytes' type elements of the the monitoring dictionary
        monitoring_d = convert_bytes_to_str(
            ecal_core.mon_monitoring(), handle_bytes="decode"
        )
        monitoring = monitoring_d[1]
        print([p['pname'] for p in monitoring['processes']])

        # save previous process state
        db_handler.execute_command("DELETE FROM previous_processes")
        db_handler.execute_command("DELETE FROM previous_services")
        db_handler.execute_command("DELETE FROM previous_topics")

        db_handler.execute_command(
            "INSERT INTO previous_processes SELECT * FROM current_processes"
        )
        db_handler.execute_command(
            "INSERT INTO previous_services SELECT * FROM current_services"
        )
        db_handler.execute_command(
            "INSERT INTO previous_topics SELECT * FROM current_topics"
        )
        # update current process state
        db_handler.execute_command("DELETE FROM current_processes")
        db_handler.execute_command("DELETE FROM current_services")
        db_handler.execute_command("DELETE FROM current_topics")

        db_handler.insert_processes(monitoring["processes"], "processes")
        db_handler.insert_processes(monitoring["processes"], "current_processes")

        db_handler.insert_services(monitoring["services"], "services")
        db_handler.insert_services(monitoring["services"], "current_services")

        db_handler.insert_topics(monitoring["topics"], "topics")
        db_handler.insert_topics(monitoring["topics"], "current_topics")

        # check for dropped processes and add them to DB
        db_handler.execute_command('''INSERT INTO dropped_processes 
                  SELECT previous_processes.* FROM previous_processes 
                  LEFT JOIN current_processes ON previous_processes.pid = current_processes.pid
                  WHERE current_processes.pid IS NULL''')

        # update host traffic table
        db_handler.execute_command("DELETE FROM edges")
        db_handler.execute_command("DELETE FROM nodes")
        hosts = db_handler.execute_command("SELECT * from current_hosts").fetchall()
        db_handler.commit()

        node_dict, edge_dict = create_host_graph_list_from_topics_and_hosts(
            monitoring["topics"], hosts
        )
        node_list, edge_list = list(node_dict.values()), list(edge_dict.values())
        
        db_handler.insert_nodes(node_list, "nodes")
        db_handler.insert_edges(edge_list, "edges")

        hnames = db_handler.execute_command(
            "SELECT DISTINCT hname from current_processes"
        ).fetchall()
        hnames = {hname[0] for hname in hnames}

        removed_hosts = set(host_subscribers.keys()).difference(hnames)
        added_hosts = hnames.difference(set(host_subscribers.keys()))

        host_subscribers = {
            k: sub for k, sub in host_subscribers.items() if k not in removed_hosts
        }

        for hname in added_hosts:
            try:
                host_subscribers[hname] = ProtoSubscriber(
                    f"machine_state_{hname}", mma_pb2.State
                )
                host_subscribers[hname].set_callback(host_monitor_callback)
                print(f"Added new subscriber for host: {hname}")
            except Exception as e:
                print(f"Error setting subscriber for {hname}: {e}")

        db_handler.commit()

        # here we could write the graph structure to a json file
        #export_graph_structure(nodes=node_list, edges=edge_list, path=os.path.dirname(os.path.abspath(__file__)) + '\\graph_structure.json')

        time.sleep(1)

    # finalize eCAL monitoring API
    ecal_core.mon_finalize()

    # finalize eCAL API
    ecal_core.finalize()

    db_handler.close()


if __name__ == "__main__":
    main()
