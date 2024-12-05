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
from helpers import convert_bytes_to_str, create_host_graph_list_from_topics

from ecal.core.subscriber import ProtoSubscriber
import proto_messages.mma_pb2 as mma_pb2

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
    # db_handler.create_host_table('current_hosts')
    # db_handler.create_host_table('previous_hosts')

    db_handler.create_process_performance_table("process_performance")

    # host graph tables (dont change any names in these two tables!)
    db_handler.create_edge_table("edges")
    db_handler.create_node_table("nodes")

    # Test graph (comment out live updates in while loop to show this)
    test_edges = [
        {
            "id": "e1",
            "source": "Rear View",
            "target": "HPC Controller",
            "mainstat": "3.14 Mbit/s",
            "secondarystat": "Camera Stream",
            "thickness": 9,
            "color": "#F07D00",
        },
        {
            "id": "e2",
            "source": "HPC Controller",
            "target": "Info",
            "mainstat": "749 Kbit/s",
            "secondarystat": "System metrics",
            "thickness": 5,
            "color": "#F07D00",
        },
        {
            "id": "e3",
            "source": "Radio",
            "target": "Info",
            "mainstat": "128 Kbit/s",
            "secondarystat": "Music",
            "thickness": 2,
            "color": "#F07D00",
        },
        {
            "id": "e4",
            "source": "Phone",
            "target": "Info",
            "mainstat": "1.50 Mbit/s",
            "secondarystat": "Navigation",
            "thickness": 7,
            "color": "#F07D00",
        },
        {
            "id": "e5",
            "source": "Rear View",
            "target": "Info",
            "mainstat": "3.14 Mbit/s",
            "secondarystat": "Rear view camera",
            "thickness": 9,
            "color": "#F07D00",
        },
        {
            "id": "e6",
            "source": "Sensor",
            "target": "HPC Controller",
            "mainstat": "941 Bit/s",
            "secondarystat": "Health parameters",
            "thickness": 2,
            "color": "#F07D00",
        },
    ]
    db_handler.insert_edges(test_edges, "edges")

    test_nodes = [
        {
            "id": "Info",
            "title": "Infotainment system",
            "mainstat": "0 Bit/s",
            "color": "#1E9BD7",
            "icon": "home-alt",
            "nodeRadius": 60,
        },
        {
            "id": "Phone",
            "title": "Smartphone",
            "mainstat": "300.65 Mit/s",
            "color": "#1E9BD7",
            "icon": "mobile-android",
            "nodeRadius": 50,
        },
        {
            "id": "Radio",
            "title": "Radio",
            "mainstat": "0 Bit/s",
            "color": "#1E9BD7",
            "icon": "rss",
            "nodeRadius": 40,
        },
        {
            "id": "Rear View",
            "title": "Rear View",
            "mainstat": "42 Bit/s",
            "color": "#1E9BD7",
            "icon": "camera",
            "nodeRadius": 40,
        },
        {
            "id": "HPC Controller",
            "title": "HPC Controller",
            "mainstat": "140 Mbit/s",
            "color": "#1E9BD7",
            "icon": "calculator-alt",
            "nodeRadius": 50,
        },
        {
            "id": "Sensor",
            "title": "Multiple Sensors",
            "mainstat": "0 Bit/s",
            "color": "#1E9BD7",
            "icon": "exclamation",
            "nodeRadius": 40,
        },
    ]
    db_handler.insert_nodes(test_nodes, "nodes")

    # print eCAL entities
    while ecal_core.ok():
        # convert 'bytes' type elements of the the monitoring dictionary
        monitoring_d = convert_bytes_to_str(
            ecal_core.mon_monitoring(), handle_bytes="decode"
        )
        monitoring = monitoring_d[1]
        print(monitoring)

        # save previous process state
        db_handler.execute_command("DELETE FROM previous_processes")
        db_handler.execute_command("DELETE FROM previous_services")
        db_handler.execute_command("DELETE FROM previous_topics")
        db_handler.execute_command("DELETE FROM previous_hosts")

        db_handler.execute_command(
            "INSERT INTO previous_processes SELECT * FROM current_processes"
        )
        db_handler.execute_command(
            "INSERT INTO previous_services SELECT * FROM current_services"
        )
        db_handler.execute_command(
            "INSERT INTO previous_topics SELECT * FROM current_topics"
        )
        db_handler.execute_command(
            "INSERT INTO previous_hosts SELECT * FROM current_hosts"
        )

        # update current process state
        db_handler.execute_command("DELETE FROM current_processes")
        db_handler.execute_command("DELETE FROM current_services")
        db_handler.execute_command("DELETE FROM current_topics")
        db_handler.execute_command("DELETE FROM current_hosts")

        db_handler.insert_processes(monitoring["processes"], "processes")
        db_handler.insert_processes(monitoring["processes"], "current_processes")

        db_handler.insert_services(monitoring["services"], "services")
        db_handler.insert_services(monitoring["services"], "current_services")

        db_handler.insert_topics(monitoring["topics"], "topics")
        db_handler.insert_topics(monitoring["topics"], "current_topics")

        # check for dropped processes and add them to DB
        db_handler.execute_command(
            """INSERT INTO dropped_processes 
                  SELECT previous_processes.* FROM previous_processes 
                  LEFT JOIN current_processes ON previous_processes.pid = current_processes.pid
                  WHERE current_processes.pid IS NULL"""
        )

        # update host traffic table
        db_handler.execute_command("DELETE FROM edges")
        db_handler.execute_command("DELETE FROM nodes")

        node_list, edge_list = create_host_graph_list_from_topics(monitoring["topics"])
        db_handler.insert_nodes(node_list, "nodes")
        db_handler.insert_edges(edge_list, "edges")

        hnames = db_handler.execute_command(
            "SELECT DISTINCT hname from processes"
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
        time.sleep(1)

    # finalize eCAL monitoring API
    ecal_core.mon_finalize()

    # finalize eCAL API
    ecal_core.finalize()

    db_handler.close()


if __name__ == "__main__":
    main()
