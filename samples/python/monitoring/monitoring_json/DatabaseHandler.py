import sqlite3
import random
import os
import json
from helpers import (
    create_host_graph_list_from_topics_and_hosts,
    create_process_graph_list_from_topics,
    create_pub_sub_topic_graph_list_from_topics,
)


class DatabaseHandler:
    def __init__(
        self,
        db_string=os.path.dirname(os.path.abspath(__file__)) + "\\ecal_monitoring.db",
    ):
        self.conn = sqlite3.connect(db_string, check_same_thread=False, timeout=3)
        self.cursor = self.conn.cursor()
        self.cursor.execute("PRAGMA journal_mode=WAL;")

    def create_log_table(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            time_stamp TEXT,
            message TEXT,
            level TEXT)
        """
        )

    def create_process_table(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            time_stamp TEXT,
            pid INTEGER,
            rclock INTEGER,
            hname TEXT,
            pname TEXT,
            uname TEXT,
            pparam TEXT,
            pmemory INTEGER,
            pcpu INTEGER,
            usrptime INTEGER,
            datawrite INTEGER,
            state_severity INTEGER,
            state_severity_level INTEGER,
            state_info TEXT,
            tsync_state INTEGER,
            tsync_mod_name TEXT,
            component_init_state INTEGER,
            component_init_info TEXT)
        """
        )

    def create_service_table(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """( 
            time_stamp TEXT,
            pid INTEGER,
            rclock INTEGER,
            hname TEXT,
            pname TEXT,
            uname TEXT,
            sname TEXT)
        """
        )

    def create_topic_table(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            time_stamp TEXT,
            pid INTEGER,
            rclock INTEGER,
            hname TEXT,
            pname TEXT,
            uname TEXT,
            tid INTEGER,
            tname TEXT,
            direction TEXT,
            ttype TEXT,
            tdesc TEXT,
            tsize INTEGER,
            dclock INTEGER,
            dfreq INTEGER,
            throughput REAL,
            connections_loc INTEGER,  
            connections_ext INTEGER,
            message_drops INTEGER,
            latency_min INTEGER,
            latency_avg INTEGER,
            latency_max INTEGER)
        """
        )

    def create_host_table(self, table_name): 
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            time_stamp TEXT,
            hname TEXT,
            pid INTEGER, 
            cpu_load REAL,
            total_memory INTEGER,
            available_memory INTEGER,
            capacity_disk INTEGER,
            available_disk INTEGER)
        """
        )

    def create_edge_table(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            id TEXT PRIMARY KEY,
            source TEXT,
            target TEXT,
            mainstat REAL,
            secondarystat REAL,
            thickness INTEGER, 
            color TEXT)
        """
        )

    def create_edge_table_other_dtypes(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            id TEXT PRIMARY KEY,
            source TEXT,
            target TEXT,
            mainstat TEXT,
            secondarystat TEXT,
            thickness INTEGER, 
            color TEXT,
            highlighted REAL)
        """
        )

    def create_node_table(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            id TEXT PRIMARY KEY,
            title TEXT,
            mainstat REAL,
            secondarystat REAL,
            arc__used_ram REAL,
            arc__free_ram REAL,
            color TEXT, 
            icon TEXT,
            nodeRadius INTEGER)
        """
        )

    def create_node_table_other_dtypes(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            id TEXT PRIMARY KEY,
            title TEXT,
            mainstat TEXT,
            secondarystat TEXT,
            arc__used_ram REAL,
            arc__free_ram REAL,
            color TEXT, 
            icon TEXT,
            nodeRadius INTEGER,
            highlighted TEXT)
        """
        )

    def create_process_performance_table(self, table_name):
        self.cursor.execute("DROP TABLE IF EXISTS " + table_name)
        self.cursor.execute(
            """
            CREATE TABLE """
            + table_name
            + """(
            time_stamp TEXT,
            hname TEXT,
            pid INTEGER,
            current_working_set_size INTEGER,
            peak_working_set_size INTEGER,
            cpu_kernel_time INTEGER, 
            cpu_user_time INTEGER, 
            cpu_creation_time INTEGER, 
            cpu_load REAL)
        """
        )

    def insert_logs(self, logs, table_name):
        query = (
            """
            INSERT INTO """
            + table_name
            + """ (
            time_stamp,
            message,
            level) 
            VALUES (datetime('now', 'localtime'), ?, ?)
            """
        )
        data = [
            (
                log["message"],
                log["level"],
            )
            for log in logs
        ]
        self.cursor.executemany(query, data)

    def insert_process(self, process, table_name):
        self.cursor.execute(
            """
            INSERT INTO """
            + table_name
            + """ (
            time_stamp,
            pid,
            rclock,
            hname,
            pname,
            uname,
            pparam,
            pmemory,
            pcpu,
            usrptime,
            datawrite,
            state_severity,
            state_severity_level,
            state_info,
            tsync_state,
            tsync_mod_name,
            component_init_state,
            component_init_info) 
            VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                process["pid"],
                process["rclock"],
                process["hname"],
                process["pname"],
                process["uname"],
                process["pparam"],
                process.get("pmemory", -1),
                process.get("pcpu", -1),
                process.get("usrptime", -1),
                process.get("datawrite", -1),
                process["state_severity"],
                process["state_severity_level"],
                process["state_info"],
                process["tsync_state"],
                process["tsync_mod_name"],
                process["component_init_state"],
                process["component_init_info"],
            ),
        )

    def insert_processes(self, processes, table_name):
        query = (
            """
            INSERT INTO """
            + table_name
            + """ (
            time_stamp,
            pid,
            rclock,
            hname,
            pname,
            uname,
            pparam,
            pmemory,
            pcpu,
            usrptime,
            datawrite,
            state_severity,
            state_severity_level,
            state_info,
            tsync_state,
            tsync_mod_name,
            component_init_state,
            component_init_info) 
            VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """
        )
        data = [
            (
                process["pid"],
                process["rclock"],
                process["hname"],
                process["pname"],
                process["uname"],
                process["pparam"],
                process.get("pmemory", -1),
                process.get("pcpu", -1),
                process.get("usrptime", -1),
                process.get("datawrite", -1),
                process["state_severity"],
                process["state_severity_level"],
                process["state_info"],
                process["tsync_state"],
                process["tsync_mod_name"],
                process["component_init_state"],
                process["component_init_info"],
            )
            for process in processes
        ]
        self.cursor.executemany(query, data)

    def insert_service(self, service, table_name):
        self.cursor.execute(
            """
            INSERT INTO """
            + table_name
            + """ (
            time_stamp,
            pid,
            rclock,
            hname,
            pname,
            uname,
            sname)
            VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?)
            """,
            (
                service["pid"],
                service["rclock"],
                service["hname"],
                service["pname"],
                service["uname"],
                service["sname"],
            ),
        )

    def insert_services(self, services, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """ (
            time_stamp,
            pid,
            rclock,
            hname,
            pname,
            uname,
            sname
        ) VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                service["pid"],
                service["rclock"],
                service["hname"],
                service["pname"],
                service["uname"],
                service["sname"],
            )
            for service in services
        ]
        self.cursor.executemany(query, data)

    def insert_topic(self, topic, table_name):
        self.cursor.execute(
            """
            INSERT INTO """
            + table_name
            + """ (
            time_stamp,
            pid,
            rclock,
            hname,
            pname,
            uname,
            tid,
            tname,
            direction,
            ttype,
            tdesc,
            tsize,
            dclock,
            dfreq,
            throughput,
            connections_loc,
            connections_ext,
            message_drops,
            latency_min,
            latency_avg,
            latency_max)
            VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )
            """,
            (
                topic["pid"],
                topic["rclock"],
                topic["hname"],
                topic["pname"],
                topic["uname"],
                topic["tid"],
                topic["tname"],
                topic["direction"],
                topic.get("ttype", ""),  # topic['ttype'],
                topic.get("tdesc", ""),  # topic['tdesc'],
                topic["tsize"],
                topic["dclock"],
                topic["dfreq"],
                (topic["dfreq"] / 1000 * topic["tsize"]),  # troughput
                topic["connections_loc"],
                topic["connections_ext"],
                topic["message_drops"],
                random.randint(1, 20),
                random.randint(21, 50),
                random.randint(51, 120),
            ),
        )

    def insert_topics(self, topics, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """ (
            time_stamp,
            pid,
            rclock,
            hname,
            pname,
            uname,
            tid,
            tname,
            direction,
            ttype,
            tdesc,
            tsize,
            dclock,
            dfreq,
            throughput,
            connections_loc,
            connections_ext,
            message_drops,
            latency_min,
            latency_avg,
            latency_max) 
        VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                topic["pid"],
                topic["rclock"],
                topic["hname"],
                topic["pname"],
                topic["uname"],
                topic["tid"],
                topic["tname"],
                topic["direction"],
                topic.get("ttype", ""),
                topic.get("tdesc", ""),
                topic["tsize"],
                topic["dclock"],
                topic["dfreq"],
                (topic["dfreq"] / 1000 * topic["tsize"]),  # throughput
                topic["connections_loc"],
                topic["connections_ext"],
                topic["message_drops"],
                random.randint(1, 20),  # latency_min
                random.randint(21, 50),  # latency_avg
                random.randint(51, 120),  # latency_max
            )
            for topic in topics
        ]
        self.cursor.executemany(query, data)

    def insert_host(self, hname, host, table_name):
        self.cursor.execute(
            """INSERT INTO """
            + table_name
            + """(
            time_stamp,
            hname,
            cpu_load,
            total_memory,
            available_memory,
            capacity_disk,
            available_disk)
            VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?)
            """,
            (
                hname,
                host.get("cpuLoad", -1),
                host["memory"]["total"],
                host["memory"]["available"],
                host["disks"][0]["capacity"],  # decide on which disk(s)
                host["disks"][0]["available"],  # decide on which disk(s)
            ),
        )

    def insert_hosts(self, hosts, table_name):
        query = (
            """INSERT INTO """
            + table_name
            + """(
            time_stamp,
            hname,
            cpu_load,
            total_memory,
            available_memory,
            capacity_disk,
            available_disk)
            VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?)
            """
        )
        data = [
            (
                host["hname"],
                host["cpuLoad"],
                host["total_memory"],
                host["available_memory"],
                host["capacity_disk"],
                host["available_disk"],
            )
            for host in hosts
        ]
        self.cursor.executemany(query, data)

    def insert_edge(self, edge, table_name):
        self.cursor.execute(
            """INSERT INTO """
            + table_name
            + """(id, source, target, mainstat, secondarystat, thickness, color) 
                 VALUES (?, ?, ?, ?, ?, ?, ?)""",
            (
                edge["id"],
                edge["source"],
                edge["target"],
                edge["mainstat"],
                edge["secondarystat"],
                edge["thickness"],
                edge["color"],
            ),
        )

    def insert_edges(self, edges, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """(id, source, target, mainstat, secondarystat, thickness, color) 
        VALUES (?, ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                edge["id"],
                edge["source"],
                edge["target"],
                edge["mainstat"],
                edge["secondarystat"],
                edge["thickness"],
                edge["color"],
            )
            for edge in edges
        ]
        self.cursor.executemany(query, data)

    def insert_edges_new(self, edges, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """(id, source, target, mainstat, secondarystat, thickness, color, highlighted) 
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                edge["id"],
                edge["source"],
                edge["target"],
                edge["mainstat"],
                edge["secondarystat"],
                edge["thickness"],
                edge["color"],
                edge["highlighted"],
            )
            for edge in edges
        ]
        self.cursor.executemany(query, data)

    def insert_node(self, node, table_name):
        self.cursor.execute(
            """INSERT INTO """
            + table_name
            + """(id, title, mainstat, secondarystat, arc__used_ram, arc__free_ram, color, icon, nodeRadius)
                     VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)""",
            (
                node["title"],
                node["id"],
                node["mainstat"],
                node["secondarystat"],
                node["arc__used_ram"],
                node["arc__free_ram"],
                node["color"],
                node["icon"],
                node["nodeRadius"],
            ),
        )

    def insert_nodes_new(self, nodes, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """(id, title, mainstat, secondarystat, arc__used_ram, arc__free_ram, color, icon, nodeRadius, highlighted) 
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                node["id"],
                node["title"],
                node["mainstat"],
                node["secondarystat"],
                node["arc__used_ram"],
                node["arc__free_ram"],
                node["color"],
                node["icon"],
                node["nodeRadius"],
                node["highlighted"],
            )
            for node in nodes
        ]
        self.cursor.executemany(query, data)

    def insert_nodes(self, nodes, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """(id, title, mainstat, secondarystat, arc__used_ram, arc__free_ram, color, icon, nodeRadius) 
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                node["id"],
                node["title"],
                node["mainstat"],
                node["secondarystat"],
                node["arc__used_ram"],
                node["arc__free_ram"],
                node["color"],
                node["icon"],
                node["nodeRadius"],
            )
            for node in nodes
        ]
        self.cursor.executemany(query, data)

    def insert_process_performance(self, hname, process, table_name):
        memory = process["memory"]
        cpu = process["cpu"]
        self.cursor.execute(
            """INSERT INTO """
            + table_name
            + """(
            time_stamp,
            hname,
            pid,
            current_working_set_size,
            peak_working_set_size,
            cpu_kernel_time,
            cpu_user_time,
            cpu_creation_time,
            cpu_load)
            VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                hname,
                process["id"],
                memory.get("currentWorkingSetSize", -1),
                memory.get("peakWorkingSetSize", -1),
                cpu.get("cpuKernelTime", -1),
                cpu.get("cpuUserTime", -1),
                cpu.get("cpuCreationTime", -1),
                cpu.get("cpuLoad", -1),
            ),
        )

    def insert_process_performances(self, hname, processes, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """(
            time_stamp,
            hname,
            pid,
            current_working_set_size,
            peak_working_set_size,
            cpu_kernel_time,
            cpu_user_time,
            cpu_creation_time,
            cpu_load)
        VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                hname,
                process["id"],
                process["memory"].get("currentWorkingSetSize", -1),
                process["memory"].get("peakWorkingSetSize", -1),
                process["cpu"].get("cpuKernelTime", -1),
                process["cpu"].get("cpuUserTime", -1),
                process["cpu"].get("cpuCreationTime", -1),
                process["cpu"].get("cpuLoad", -1),
            )
            for process in processes
        ]
        self.cursor.executemany(query, data)

    def insert_process_performances_new(self, processes, table_name):
        query = (
            """
        INSERT INTO """
            + table_name
            + """(
            time_stamp,
            hname,
            pid,
            current_working_set_size,
            peak_working_set_size,
            cpu_kernel_time,
            cpu_user_time,
            cpu_creation_time,
            cpu_load)
        VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?)
    """
        )
        data = [
            (
                process["hname"],
                process["id"],
                process["currentWorkingSetSize"],
                process["peakWorkingSetSize"],
                process["cpuKernelTime"],
                process["cpuUserTime"],
                process["cpuCreationTime"],
                process["cpuLoad"],
            )
            for process in processes
        ]
        self.cursor.executemany(query, data)

    def execute_command(self, command):
        return self.cursor.execute(command)

    def execute_command_with_args(self, command, args):
        return self.cursor.execute(command, args)

    def commit(self):
        self.conn.commit()

    def close(self):
        self.conn.close()


if __name__ == "__main__":
    db_handler = DatabaseHandler()
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
    db_handler.create_host_table("previous_hosts")

    db_handler.create_edge_table("edges")
    db_handler.create_node_table("nodes")

    db_handler.create_process_performance_table("process_performance")

    process = {
        "rclock": 4,
        "hname": "testname",
        "hgname": "testname",
        "pid": 30524,
        "pname": "C:\\Program Files\\WindowsApps\\PythonSoftwareFoundation.Python.3.12_3.12.2288.0_x64__qbz5n2kfra8p0\\python3.12.exe",
        "uname": "python3.12",
        "pparam": '"C:\\Users\\testname\\AppData\\Local\\Microsoft\\WindowsApps\\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\\python.exe" samples\\python\\monitoring\\monitoring_json\\monitoring_json.py ',
        "state_severity": 0,
        "state_severity_level": 1,
        "state_info": "",
        "tsync_state": 0,
        "tsync_mod_name": "ecaltime-localtime",
        "component_init_state": 63,
        "component_init_info": "pub|sub|log|time",
        "ecal_runtime_version": "v6.0.0-alpha0-135-ga5dc7ca9c",
    }
    # db_handler.insert_process(process, "processes")

    service = {
        "rclock": 1,
        "hname": "testname",
        "pname": "C:\\Program Files\\WindowsApps\\PythonSoftwareFoundation.Python.3.12_3.12.2288.0_x64__qbz5n2kfra8p0\\python3.12.exe",
        "uname": "python3.12",
        "pid": 23164,
        "sname": "DemoService",
        "sid": "429631908349000",
        "version": 0,
        "tcp_port_v0": 0,
        "tcp_port_v1": 51766,
        "methods": {},
    }
    # db_handler.insert_service(service, "services")

    topic = {
        "rclock": 4,
        "hname": "testname",
        "hgname": "testname",
        "pid": 30524,
        "pname": "C:\\Program Files\\WindowsApps\\PythonSoftwareFoundation.Python.3.12_3.12.2288.0_x64__qbz5n2kfra8p0\\python3.12.exe",
        "uname": "python3.12",
        "tid": "429127997510400",
        "tname": "hello_world_python_protobuf_topic",
        "direction": "publisher",
        "tsize": 0,
        "connections_loc": 1,
        "connections_ext": 0,
        "message_drops": 0,
        "did": 0,
        "dclock": 0,
        "dfreq": 0,
    }
    # db_handler.insert_topic(topic, "topics")

    host = {
        "cpuLoad": 4.85774430741831,
        "memory": {"total": "12341234123412341234124124", "available": "12312431234"},
        "disks": [
            {
                "name": "C:",
                "write": 1439722.5077521738,
                "available": "213342134213",
                "capacity": "123443234122",
            }
        ],
        "networks": [
            {
                "name": "Intel[R] Ethernet Connection [23] I219-LM",
                "ipAddress": "192.168.178.174",
            }
        ],
        "numberOfCpuCores": 8,
        "operatingSystem": "WINDOWS 64BITS",
        "process": [
            {
                "name": "C:\\eCAL\\bin\\ecal_mma.exe",
                "id": 15472,
                "commandline": '"C:\\eCAL\\bin\\ecal_mma.exe" ',
                "memory": {
                    "currentWorkingSetSize": "17874944",
                    "peakWorkingSetSize": "22450176",
                },
                "cpu": {
                    "cpuKernelTime": "3362812500",
                    "cpuUserTime": "820156250",
                    "cpuCreationTime": "133776119801608063",
                },
            }
        ],
    }
    hname = "testname"
    # db_handler.insert_host(hname, host, "hosts")

    edge = {
        "id": "testname" + "_" + "testname2",
        "source": "testname",
        "target": "testname2",
        "mainstat": 12341234,
        "secondarystat": "",
        "thickness": 1,
        "color": "#F07D00",
    }
    # db_handler.insert_edge(edge, "edges")

    node = {
        "id": "testname",
        "title": "testname",
        "mainstat": 12341234,
        "color": "#1E9BD7",
        "icon": "",
        "nodeRadius": 50,
        "secondarystat": 2,
        "arc__used_ram": 0.4,
        "arc__free_ram": 0.6,
    }
    # db_handler.insert_node(node, "nodes")

    process = {
        "name": "C:\\eCAL\\bin\\ecal_mma.exe",
        "id": 15472,
        "commandline": '"C:\\eCAL\\bin\\ecal_mma.exe" ',
        "memory": {
            "currentWorkingSetSize": "17874944",
            "peakWorkingSetSize": "22450176",
        },
        "cpu": {
            "cpuKernelTime": "3362812500",
            "cpuUserTime": "820156250",
            "cpuCreationTime": "133776119801608063",
        },
    }
    hname = "testname"
    # db_handler.insert_process_performance(hname, process, "process_performance")

    with open(
        "C:\\Users\\d93445\\Desktop\\ecal\\samples\\python\\monitoring\\monitoring_json\\topics_hosts_to_test_graph.json",
        "r",
    ) as file:
        data = json.load(file)

    # Print the data
    # print(data)
    node_list, edge_list = create_host_graph_list_from_topics_and_hosts(
        data["topics"], data["hosts"]
    )
    # print([node['id'] for node in node_list])
    # print([edge['id'] for edge in edge_list])
    # print(node_list)

    # print(node_list)
    db_handler.insert_nodes(list(node_list.values()), "nodes")
    db_handler.insert_edges(list(edge_list.values()), "edges")

    # test for the process graph
    db_handler.create_edge_table_other_dtypes("pg_edges")
    db_handler.create_node_table_other_dtypes("pg_nodes")

    node_dict, edge_dict = create_process_graph_list_from_topics(data["topics"])
    db_handler.insert_nodes_new(list(node_dict.values()), "pg_nodes")
    db_handler.insert_edges_new(list(edge_dict.values()), "pg_edges")

    # test for the pub-sub-topic graph
    db_handler.create_edge_table_other_dtypes("pstg_edges")
    db_handler.create_node_table_other_dtypes("pstg_nodes")

    node_dict, edge_dict = create_pub_sub_topic_graph_list_from_topics(data["topics"])
    db_handler.insert_nodes_new(list(node_dict.values()), "pstg_nodes")
    db_handler.insert_edges_new(list(edge_dict.values()), "pstg_edges")

    db_handler.commit()
    db_handler.close()
