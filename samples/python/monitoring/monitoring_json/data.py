from helpers import create_host_graph_list_from_topics_and_hosts_new
from ecal.core.subscriber import ProtoSubscriber
import proto_messages.mma_pb2 as mma_pb2
from DatabaseHandler import DatabaseHandler
from helpers import convert_bytes_to_str
from google.protobuf.json_format import MessageToDict
from collections import defaultdict


def host_monitor_callback_no_db(topic_name, msg, time):
    monitor = Monitor()

    hname = topic_name.split("_")[2]
    host = MessageToDict(message=msg)
    monitor.update_host(hname, host)

    ecal_pids = {pid for pid, p in monitor.processes.items() if p["hname"] == hname}
    ecal_processes = [p for p in host["process"] if p["id"] in ecal_pids]    
    monitor.update_processes_information(hname, ecal_processes)


def singleton(cls):
    instances = {}

    def get_instance(*args, **kwargs):
        if cls not in instances:
            instances[cls] = cls(*args, **kwargs)
        return instances[cls]

    return get_instance


@singleton
class Monitor:
    def __init__(self):
        self.processes = {}  # "current"
        self.prvious_processes = {}
        self.dropped_processes = {}
        self.services = {}
        self.previous_services = {}
        self.topics = {}
        self.previous_topics = {}
        self.hosts = {}
        self.hosts_information = {}
        self.process_performances =  defaultdict(dict)
        self.mma_subscribers = {}
        self.edges = {}
        self.nodes = {}
        self.logs = []
        self.db_handler = DatabaseHandler()
        self.setup_db()

    def setup_db(self):
        # do we really need the last ticks data in the Dashboard? We have the current and all (raw)
        self.db_handler.create_process_table("processes")
        self.db_handler.create_process_table("current_processes")
        self.db_handler.create_process_table("previous_processes")
        self.db_handler.create_process_table("dropped_processes")

        self.db_handler.create_process_performance_table("process_performance")

        self.db_handler.create_service_table("services")
        self.db_handler.create_service_table("current_services")
        self.db_handler.create_service_table("previous_services")
        
        self.db_handler.create_topic_table("topics")
        self.db_handler.create_topic_table("current_topics")
        self.db_handler.create_topic_table("previous_topics")

        self.db_handler.create_host_table("hosts")
        self.db_handler.create_edge_table("edges")
        self.db_handler.create_node_table("nodes")

        self.db_handler.create_log_table("logs")

    def update_processes(self, processes):
        # todo: check if pids are unique system wide
        self.previous_processes = self.processes
        self.processes = {
            process["pid"]: process for process in processes
        }  

        dropped_process_ids = set(self.previous_processes.keys()).difference(
            set(self.processes.keys())
        )  # to test
        self.dropped_processes = {
            pid: self.previous_processes[pid] for pid in dropped_process_ids
        }
        for dropped_process in self.dropped_processes.values():
            self.logs.append({"message": f"[DROPPED PROCESS] dropped process with id={dropped_process['pid']} on host={dropped_process['hname']}", "level": "warning"})

        current_process_ids = set(self.processes.keys())
        previous_process_ids = set(self.previous_processes.keys())
        new_process_ids = current_process_ids.difference(previous_process_ids)

        new_processes = {
            pid: self.processes[pid] for pid in new_process_ids
        }
        for new_process in new_processes.values():
            self.logs.append({"message": f"[NEW PROCESS] start monitoring process with id={new_process['pid']} on host={new_process['hname']}", "level": "info"})

    def update_services(self, services):
        self.previous_services = self.services
        self.services = {service["sname"]: service for service in services}

    def update_topics(self, topics):
        self.previous_topics = self.topics
        self.topics = {topic["tid"]: topic for topic in topics}

    def update_host(
        self, hname, host
    ):  # used by the mma callback, allows to save more details about a host
        disk = host["disks"][0]  # decide on which disk(s)
        memory = host["memory"]
        self.hosts[hname] = {
            "hname": hname,
            "cpuLoad": float(host.get("cpuLoad", -1)),
            "total_memory": int(memory["total"]),
            "available_memory": int(memory["available"]),
            "capacity_disk": int(disk["capacity"]),
            "available_disk": int(disk["available"]),
        }

    def update_processes_information(self, hname, processes):
        for process in processes:
            memory = process["memory"]
            cpu = process["cpu"]

            process_information = {
                "hname": hname,
                "id": process["id"],
                "currentWorkingSetSize": int(memory.get("currentWorkingSetSize", -1)),# check is type correct?
                "peakWorkingSetSize": int(memory.get("peakWorkingSetSize", -1)),# check is type correct?
                "cpuKernelTime": int(cpu.get("cpuKernelTime", -1)),# check is type correct?
                "cpuUserTime": int(cpu.get("cpuUserTime", -1)),# check is type correct?
                "cpuCreationTime": int(cpu.get("cpuCreationTime", -1)),# check is type correct?
                "cpuLoad": float(cpu.get("cpuLoad", -1)),# check is type correct?
            }

            self.process_performances[hname][process["id"]] = process_information

    def update_graph(self):
        hosts = defaultdict(lambda: {"disk_usage": 0, "ram_usage": 0})
        for hname, host_dict in self.hosts.items():
            hosts[hname] = {
                "disk_usage": round(number=(host_dict["capacity_disk"] - host_dict["available_disk"]) / host_dict["capacity_disk"], ndigits=3),
                "ram_usage": round(number=(host_dict["total_memory"] - host_dict["available_memory"]) / host_dict["total_memory"], ndigits=2),
            }
        self.nodes, self.edges = create_host_graph_list_from_topics_and_hosts_new(
            list(self.topics.values()), hosts
        )

    def update_mma_subscribers(self):
        # log for new host/ dropped host 
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
                self.mma_subscribers[hname].set_callback(host_monitor_callback_no_db)
                self.logs.append({"message": f"[NEW HOST] start monitoring host {hname}", "level": "info"})

                print(f"Added new subscriber for host: {hname}")
            except Exception as e:
                print(f"Error setting subscriber for {hname}: {e}")
                self.logs.append({"message": f"[STOPPED HOST] stop monitoring host {hname}", "level": "critical"})


    def write_to_db(self):
        self.db_handler.execute_command("DELETE FROM current_processes")
        self.db_handler.execute_command("DELETE FROM current_services")
        self.db_handler.execute_command("DELETE FROM current_topics")

        self.db_handler.execute_command("DELETE FROM previous_processes")
        self.db_handler.execute_command("DELETE FROM previous_services")
        self.db_handler.execute_command("DELETE FROM previous_topics")

        self.db_handler.insert_processes(list(self.processes.values()), "processes")
        self.db_handler.insert_services(list(self.services.values()), "services")
        self.db_handler.insert_topics(list(self.topics.values()), "topics")

        self.db_handler.insert_processes(list(self.processes.values()), "current_processes")
        self.db_handler.insert_services(list(self.services.values()), "current_services")
        self.db_handler.insert_topics(list(self.topics.values()), "current_topics")
        
        self.db_handler.insert_processes(list(self.previous_processes.values()), "previous_processes")
        self.db_handler.insert_services(list(self.previous_services.values()), "previous_services")
        self.db_handler.insert_topics(list(self.previous_topics.values()), "previous_topics")

        self.db_handler.insert_hosts(list(self.hosts.values()), "hosts")

        for _, process_perf_dict in self.process_performances.items():
            self.db_handler.insert_process_performances_new(list(process_perf_dict.values()), "process_performance")
    

        if self.dropped_processes:  # similar for new processes + add "log" Table
            self.db_handler.insert_processes(
                list(self.dropped_processes.values()), "dropped_processes"
            )

        self.db_handler.execute_command("DELETE FROM edges")
        self.db_handler.execute_command("DELETE FROM nodes")

        self.db_handler.insert_nodes(list(self.nodes.values()), "nodes")
        self.db_handler.insert_edges(list(self.edges.values()), "edges")

        self.db_handler.insert_logs(self.logs, "logs")
        self.logs = []


        self.db_handler.commit()

    def update_monitor(self, ecal_data):
        monitoring_d = convert_bytes_to_str(ecal_data, handle_bytes="decode")
        monitoring = monitoring_d[1]

        self.update_processes(monitoring["processes"])
        self.update_topics(monitoring["topics"])
        self.update_services(monitoring["services"])
        self.update_graph()
        self.update_mma_subscribers()
        self.write_to_db()

    def finalize_monitor(self):
        self.db_handler.close()


if __name__ == "__main__":
    m1 = Monitor()
    m2 = Monitor()
    print(m1 == m2)
