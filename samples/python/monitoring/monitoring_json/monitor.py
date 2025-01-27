from helpers import (
    create_host_graph,
    create_pub_sub_topic_graph,
    create_process_graph,
    create_client_server_graph,
)
from ecal.core.subscriber import ProtoSubscriber
import proto_messages.mma_pb2 as mma_pb2
from helpers import convert_bytes_to_str
from google.protobuf.json_format import MessageToDict
from collections import defaultdict

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
import os
from models import *


class Monitor:
    def __init__(self, relative_db_path="db/ecal_monitoring.db"):
        self.processes = {}  # "current"
        self.prvious_processes = {}
        self.dropped_processes = {}
        self.services = {}
        self.previous_services = {}
        self.clients = {}
        self.previous_clients = {}
        self.topics = {}
        self.previous_topics = {}
        self.hosts = {}
        self.hosts_information = {}
        self.process_performances = defaultdict(dict)
        self.mma_subscribers = {}
        self.host_edges = {}
        self.host_nodes = {}
        self.pub_sub_topic_edges = {}
        self.pub_sub_topic_nodes = {}
        self.process_edges = {}
        self.process_nodes = {}
        self.client_server_edges = {}
        self.client_server_nodes = {}
        self.logs = []

        db_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), relative_db_path)
        )
        db_uri = f"sqlite:///{db_path}"

        self.engine = create_engine(db_uri)
        self.Session = sessionmaker(bind=self.engine)
        Base.metadata.drop_all(self.engine)
        Base.metadata.create_all(self.engine)

    def update_processes(self, processes):
        self.previous_processes = self.processes
        self.processes = {process["pid"]: process for process in processes}

        for pid, dropped_process in self.previous_processes.items():
            if pid not in self.processes:
                self.dropped_processes[pid] = dropped_process
                self.logs.append(
                    {
                        "message": f"[DROPPED PROCESS] dropped process with id={dropped_process['pid']} and uname=\"{dropped_process['uname']}\" on host={dropped_process['hname']}",
                        "level": "warning",
                    }
                )

        for pid, new_process in self.processes.items():
            if pid not in self.previous_processes:
                self.logs.append(
                    {
                        "message": f"[NEW PROCESS] start monitoring process with id={new_process['pid']} and uname=\"{new_process['uname']}\" on host={new_process['hname']}",
                        "level": "info",
                    }
                )

    def update_services(self, services):
        self.previous_services = self.services
        self.services = {service["sname"]: service for service in services}

        for sname, service in self.services.items():
            if sname not in self.previous_services:
                self.logs.append(
                    {
                        "message": f"[NEW SERVICE] with pid={service['pid']} and sname=\"{service['sname']}\" on host={service['hname']}",
                        "level": "info",
                    }
                )

        for sname, service in self.previous_services.items():
            if sname not in self.services:
                self.logs.append(
                    {
                        "message": f"[DROPPED SERVICE] with pid={service['pid']} and sname=\"{service['sname']}\" on host={service['hname']}",
                        "level": "warning",
                    }
                )

    def update_clients(self, clients):
        self.previous_clients = self.clients
        self.clients = {client["sid"]: client for client in clients}

        for sid, client in self.clients.items():
            if sid not in self.previous_clients:
                self.logs.append(
                    {
                        "message": f"[NEW CLIENT] with pid=\"{client['pid']}\" and uname=\"{client['uname']}\" on host={client['hname']}",
                        "level": "info",
                    }
                )

        for sid, client in self.previous_clients.items():
            if sid not in self.clients:
                self.logs.append(
                    {
                        "message": f"[DROPPED CLIENT] with pid=\"{client['pid']}\" and uname=\"{client['uname']}\" on host={client['hname']}",
                        "level": "warning",
                    }
                )

    def update_topics(self, topics):
        self.previous_topics = self.topics
        for topic in topics:
            topic["layer"] = next((item['type'] for item in topic['layer'] if item['active']), None) # it should be exactly 1 layer true per topic, currently verifying
            topic["throughput"] = int((topic["dfreq"] / 1000 * topic["tsize"])/1000) # should be kBps
        self.topics = {topic["tid"]: topic for topic in topics}

        for tid, topic in self.topics.items():
            if tid not in self.previous_topics:
                self.logs.append(
                    {
                        "message": f"[NEW {topic['direction'].upper()}] for topic \"{topic['tname']}\" with uname=\"{topic['uname']}\" on host={topic['hname']}",
                        "level": "info",
                    }
                )

        for tid, topic in self.previous_topics.items():
            if tid not in self.topics:
                self.logs.append(
                    {
                        "message": f"[STOPPED {topic['direction'].upper()}] for topic \"{topic['tname']}\" with uname=\"{topic['uname']}\" on host={topic['hname']}",
                        "level": "warning",
                    }
                )

    def update_host(self, hname, host):
        # extend for multiple disks and networks
        disks = host.get("disks", [])
        disk = disks[0] if disks else None

        networks = host.get("networks", [])
        network = networks[0] if networks else None

        memory = host.get("memory", {})
        self.hosts[hname] = {
            "hname": hname,
            "cpu_load": float(host.get("cpuLoad", -1)),
            "total_memory": int(memory.get("total", -1)),
            "available_memory": int(memory.get("available", -1)),
            "capacity_disk": int(disk["capacity"]) if disk else -1,
            "available_disk": int(disk["available"]) if disk else -1,
            "network_send": int(network.get("send", -1)) if network else -1,
            "network_receive": int(network.get("receive", -1)) if network else -1,
            "os": host["operatingSystem"],
            "num_cpu_cores": host["numberOfCpuCores"],
        }

    def update_processes_information(self, hname, processes):
        for process in processes:
            memory = process["memory"]
            cpu = process["cpu"]

            process_information = {
                "hname": hname,
                "pid": process["id"],
                "current_working_set_size": int(
                    memory.get("currentWorkingSetSize", -1)
                ),
                "peak_working_set_size": int(memory.get("peakWorkingSetSize", -1)),
                "cpu_kernel_time": int(cpu.get("cpuKernelTime", -1)),
                "cpu_user_time": int(cpu.get("cpuUserTime", -1)),
                "cpu_creation_time": int(cpu.get("cpuCreationTime", -1)),
                "cpu_load": float(cpu.get("cpuLoad", -1)),
            }

            self.process_performances[hname][process["id"]] = process_information

    def update_host_graph(self):
        hosts = defaultdict(lambda: {"disk_usage": 0, "ram_usage": 0, "cpu_load": 0})
        for hname, host_dict in self.hosts.items():
            hosts[hname] = {
                "disk_usage": round(
                    number=(host_dict["capacity_disk"] - host_dict["available_disk"])
                    / host_dict["capacity_disk"],
                    ndigits=3,
                ),
                "ram_usage": round(
                    number=(host_dict["total_memory"] - host_dict["available_memory"])
                    / host_dict["total_memory"],
                    ndigits=2,
                ),
                "cpu_load": round(number=host_dict["cpu_load"]/100, ndigits=2)
            }
        self.host_nodes, self.host_edges = create_host_graph(
            list(self.topics.values()), hosts
        )

    def update_topic_graph(self):
        self.pub_sub_topic_nodes, self.pub_sub_topic_edges = create_pub_sub_topic_graph(
            list(self.topics.values()),
            self.process_performances,
        )

    def update_process_graph(self):
        self.process_nodes, self.process_edges = create_process_graph(
            list(self.topics.values()),
            self.process_performances,
        )

    def update_client_server_graph(self):
        self.client_server_nodes, self.client_server_edges = create_client_server_graph(
            list(self.clients.values()),
            list(self.services.values()),
            self.process_performances,
        )

    def host_monitor_callback_no_db(self, topic_name, msg, time):
        hname = topic_name.split("_")[2]
        host = MessageToDict(message=msg)
        #print(host.keys())
        #print(host['disks'])
        #print(host['networks'])
        self.update_host(hname, host)

        ecal_pids = {pid for pid, p in self.processes.items() if p["hname"] == hname}
        ecal_processes = [p for p in host["process"] if p["id"] in ecal_pids]
        self.update_processes_information(hname, ecal_processes)

    def update_mma_subscribers(self):
        hnames = {process["hname"] for process in self.processes.values()}
        removed_hosts = set(self.mma_subscribers.keys()).difference(hnames)
        added_hosts = hnames.difference(set(self.mma_subscribers.keys()))

        self.mma_subscribers = {
            k: sub for k, sub in self.mma_subscribers.items() if k not in removed_hosts
        }

        for hname in added_hosts:
            try:
                self.mma_subscribers[hname] = ProtoSubscriber(
                    f"machine_state_{hname}", mma_pb2.State
                )
                self.mma_subscribers[hname].set_callback(self.host_monitor_callback_no_db)
                self.logs.append(
                    {
                        "message": f"[NEW HOST] start monitoring host {hname}",
                        "level": "info",
                    }
                )

                print(f"Added new subscriber for host: {hname}")
            except Exception as e:
                print(f"Error setting subscriber for {hname}: {e}")
                self.logs.append(
                    {
                        "message": f"[STOPPED HOST] stop monitoring host {hname}",
                        "level": "critical",
                    }
                )

    def write_to_db(self):
        with self.Session() as session:
            session.query(CurrentProcess).delete()
            session.query(CurrentService).delete()
            session.query(CurrentTopic).delete()
            session.query(CurrentClient).delete()
            session.query(PreviousProcess).delete()
            session.query(PreviousService).delete()
            session.query(PreviousClient).delete()
            session.query(PreviousTopic).delete()
            session.query(HostNode).delete()
            session.query(HostEdge).delete()
            session.query(PubSubTopicNode).delete()
            session.query(PubSubTopicEdge).delete()
            session.query(ProcessNode).delete()
            session.query(ProcessEdge).delete()
            session.query(ClientServerNode).delete()
            session.query(ClientServerEdge).delete()

            session.bulk_insert_mappings(Process, self.processes.values())
            session.bulk_insert_mappings(Service, self.services.values())
            session.bulk_insert_mappings(Topic, self.topics.values())
            session.bulk_insert_mappings(Client, self.clients.values())

            session.bulk_insert_mappings(CurrentProcess, self.processes.values())
            session.bulk_insert_mappings(CurrentService, self.services.values())
            session.bulk_insert_mappings(CurrentTopic, self.topics.values())
            session.bulk_insert_mappings(CurrentClient, self.clients.values())

            session.bulk_insert_mappings(
                PreviousProcess, self.previous_processes.values()
            )
            session.bulk_insert_mappings(
                PreviousService, self.previous_services.values()
            )
            session.bulk_insert_mappings(PreviousTopic, self.previous_topics.values())

            session.bulk_insert_mappings(PreviousClient, self.previous_clients.values())

            for _, process_perf_dict in self.process_performances.items():
                session.bulk_insert_mappings(
                    ProcessPerformance, process_perf_dict.values()
                )

            if self.dropped_processes:
                session.bulk_insert_mappings(
                    DroppedProcess, self.dropped_processes.values()
                )

            session.bulk_insert_mappings(Host, self.hosts.values())

            session.bulk_insert_mappings(HostNode, self.host_nodes.values())
            session.bulk_insert_mappings(HostEdge, self.host_edges.values())

            session.bulk_insert_mappings(
                PubSubTopicNode, self.pub_sub_topic_nodes.values()
            )
            session.bulk_insert_mappings(
                PubSubTopicEdge, self.pub_sub_topic_edges.values()
            )

            session.bulk_insert_mappings(ProcessNode, self.process_nodes.values())
            session.bulk_insert_mappings(ProcessEdge, self.process_edges.values())

            session.bulk_insert_mappings(
                ClientServerNode, self.client_server_nodes.values()
            )
            session.bulk_insert_mappings(
                ClientServerEdge, self.client_server_edges.values()
            )

            session.bulk_insert_mappings(Log, self.logs)
            session.commit()
        self.logs = []

    def update_monitor(self, ecal_data):
        monitoring_d = convert_bytes_to_str(ecal_data, handle_bytes="decode")
        monitoring = monitoring_d[1]

        #print(monitoring.keys())
        #print("proc  ", monitoring["processes"][0].keys())
        #print("topics  ", monitoring["topics"][0].keys())
        #print(monitoring["services"][0].keys())
        #print(monitoring["clients"][0].keys())

        self.update_processes(monitoring["processes"])
        self.update_topics(monitoring["topics"])
        self.update_services(monitoring["services"])
        self.update_clients(monitoring["clients"])

        self.update_host_graph()
        self.update_topic_graph()
        self.update_process_graph()
        self.update_client_server_graph()
        self.update_mma_subscribers()
        self.write_to_db()

    def finalize_monitor(self):
        self.session.close()
        self.engine.dispose()
