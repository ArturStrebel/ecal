from sqlalchemy import Column, Integer, Float, String, DateTime, Text, func
from sqlalchemy.orm import declarative_base
import os
import json

Base = declarative_base()


class Host(Base):
    __tablename__ = "hosts"

    time_stamp = Column(DateTime, primary_key=True, default=func.now())
    hname = Column(String, primary_key=True)
    cpu_load = Column(Float)
    total_memory = Column(Integer)
    available_memory = Column(Integer)
    capacity_disk = Column(Integer)
    available_disk = Column(Integer)
    os = Column(Text)
    num_cpu_cores = Column(Integer)

    def __repr__(self):
        return (
            f"Host(time_stamp={self.time_stamp}, hname={self.hname}, "
            f"cpu_load={self.cpu_load}, "
            f"total_memory={self.total_memory}, available_memory={self.available_memory}, "
            f"capacity_disk={self.capacity_disk}, available_disk={self.available_disk})"
        )


class AbstractProcess(Base):
    __abstract__ = True

    pid = Column(Integer, primary_key=True)
    time_stamp = Column(DateTime, primary_key=True, default=func.now())
    rclock = Column(Integer)
    hname = Column(Text)
    pname = Column(Text)
    hgname = Column(Text)
    ecal_runtime_version = Column(Text)
    uname = Column(Text)
    pparam = Column(Text)
    # pmemory = Column(Integer)
    # pcpu = Column(Integer)
    # usrptime = Column(Integer)
    # datawrite = Column(Integer)
    state_severity = Column(Integer)
    state_severity_level = Column(Integer)
    state_info = Column(Text)
    tsync_state = Column(Integer)
    tsync_mod_name = Column(Text)
    component_init_state = Column(Integer)
    component_init_info = Column(Text)

    def __repr__(self):
        return (
            f"Process(pid={self.pid}, time_stamp={self.time_stamp}, "
            f"rclock={self.rclock}, hname={self.hname}, pname={self.pname}, "
            f"uname={self.uname}, pparam={self.pparam},"
            f"state_severity={self.state_severity}, state_severity_level={self.state_severity_level}, "
            f"state_info={self.state_info}, tsync_state={self.tsync_state}, "
            f"tsync_mod_name={self.tsync_mod_name}, component_init_state={self.component_init_state}, "
            f"component_init_info={self.component_init_info})"
        )


class Process(AbstractProcess):
    __tablename__ = "processes"


class CurrentProcess(AbstractProcess):
    __tablename__ = "current_processes"


class PreviousProcess(AbstractProcess):
    __tablename__ = "previous_processes"


class DroppedProcess(AbstractProcess):
    __tablename__ = "dropped_processes"


class ProcessPerformance(Base):
    __tablename__ = "process_performance"

    id = Column(Integer, primary_key=True, autoincrement=True)
    time_stamp = Column(DateTime, default=func.now())
    pid = Column(Integer)
    hname = Column(Text)
    current_working_set_size = Column(Integer)
    peak_working_set_size = Column(Integer)
    cpu_kernel_time = Column(Integer)
    cpu_user_time = Column(Integer)
    cpu_creation_time = Column(Integer)
    cpu_load = Column(Float)

    def __repr__(self):
        return (
            f"ProcessPerformance(time_stamp={self.time_stamp}, hname={self.hname}, "
            f"pid={self.pid}, current_working_set_size={self.current_working_set_size}, "
            f"peak_working_set_size={self.peak_working_set_size}, "
            f"cpu_kernel_time={self.cpu_kernel_time}, cpu_user_time={self.cpu_user_time}, "
            f"cpu_creation_time={self.cpu_creation_time}, cpu_load={self.cpu_load})"
        )


class AbstractService(Base):
    __abstract__ = True

    time_stamp = Column(DateTime, primary_key=True, default=func.now())
    pid = Column(Integer, primary_key=True)
    rclock = Column(Integer)
    hname = Column(Text)
    pname = Column(Text)
    uname = Column(Text)
    sname = Column(Text, primary_key=True)

    def __repr__(self):
        return (
            f"Service(time_stamp={self.time_stamp}, "
            f"pid={self.pid}, rclock={self.rclock}, "
            f"hname={self.hname}, pname={self.pname}, "
            f"uname={self.uname}, sname={self.sname})"
        )


class Service(AbstractService):
    __tablename__ = "services"


class CurrentService(AbstractService):
    __tablename__ = "current_services"


class PreviousService(AbstractService):
    __tablename__ = "previous_services"


class AbstractClient(Base):
    __abstract__ = True

    time_stamp = Column(DateTime, primary_key=True, default=func.now())
    rclock = Column(Integer)
    hname = Column(Text)
    pname = Column(Text)
    uname = Column(Text)
    pid = Column(Integer)
    sname = Column(Text)
    sid = Column(Text, primary_key=True)
    version = Column(Integer)

    def __repr__(self):
        return (
            f"Client(rclock={self.rclock}, "
            f"hname={self.hname}, pname={self.pname}, "
            f"uname={self.uname}, pid={self.pid}, "
            f"sname={self.sname}, sid={self.sid}, "
            f"version={self.version})"
        )


class Client(AbstractClient):
    __tablename__ = "clients"


class PreviousClient(AbstractClient):
    __tablename__ = "previous_clients"


class CurrentClient(AbstractClient):
    __tablename__ = "current_clients"


class AbstractTopic(Base):
    __abstract__ = True

    time_stamp = Column(DateTime, default=func.now(), primary_key=True)
    pid = Column(Integer)
    rclock = Column(Integer)
    hname = Column(Text)
    pname = Column(Text)
    uname = Column(Text)
    tid = Column(Integer, primary_key=True)
    tname = Column(Text)
    direction = Column(Text)
    layer = Column(Text)
    # ttype = Column(Text)
    # tdesc = Column(Text)
    tsize = Column(Integer)
    dclock = Column(Integer)
    dfreq = Column(Integer)
    throughput = Column(Float)
    connections_loc = Column(Integer)
    connections_ext = Column(Integer)
    message_drops = Column(Integer)

    def __repr__(self):
        return (
            f"Topic(time_stamp={self.time_stamp}, pid={self.pid}, "
            f"rclock={self.rclock}, hname={self.hname}, pname={self.pname}, "
            f"uname={self.uname}, tid={self.tid}, tname={self.tname}, "
            f"direction={self.direction}"
            f"tsize={self.tsize}, dclock={self.dclock}, dfreq={self.dfreq}, "
            f"throughput={self.throughput}, connections_loc={self.connections_loc}, "
            f"connections_ext={self.connections_ext}, message_drops={self.message_drops})"
        )


class Topic(AbstractTopic):
    __tablename__ = "topics"


class CurrentTopic(AbstractTopic):
    __tablename__ = "current_topics"


class PreviousTopic(AbstractTopic):
    __tablename__ = "previous_topics"


class Log(Base):
    __tablename__ = "logs"

    id = Column(Integer, primary_key=True)
    time_stamp = Column(DateTime, default=func.now())
    message = Column(Text)
    level = Column(Text)

    def __repr__(self):
        return (
            f"Log(id={self.id}, time_stamp={self.time_stamp}, "
            f"message={self.message}, level={self.level})"
        )


class AbstractNode(Base):
    __abstract__ = True

    id = Column(String, primary_key=True)
    title = Column(String)
    subtitle = Column(String)
    mainstat = Column(Float)
    secondarystat = Column(Float)
    color = Column(String)
    icon = Column(String)
    nodeRadius = Column(Integer)

    def __repr__(self):
        return (
            f"Node(id={self.id}, title={self.title}, mainstat={self.mainstat}, "
            f"secondarystat={self.secondarystat}, arc__used_ram={self.arc__used_ram}, "
            f"arc__free_ram={self.arc__free_ram}, color={self.color}, "
            f"icon={self.icon}, nodeRadius={self.nodeRadius}, time_stamp={self.time_stamp})"
        )


class AbstractEdge(Base):
    __abstract__ = True

    id = Column(String, primary_key=True)
    source = Column(String)
    target = Column(String)
    mainstat = Column(Float)
    secondarystat = Column(Float)
    thickness = Column(Integer)
    color = Column(String)

    def __repr__(self):
        return (
            f"Edge(id={self.id}, source={self.source}, target={self.target}, "
            f"mainstat={self.mainstat}, secondarystat={self.secondarystat}, "
            f"thickness={self.thickness}, color={self.color}, "
            f"time_stamp={self.time_stamp})"
        )


class HostNode(AbstractNode):
    __tablename__ = "host_nodes"

    #arc__used_ram = Column(Float)
    #arc__free_ram = Column(Float)
    # more general health indicator: 1/3 of circle each RAM, CPU, DISK; green if ok, red otherwise, e.g., when > treshold for ex 80 %  
    #arc__ram_ok = Column(Float)
    #arc__ram_not_ok = Column(Float)
    #arc__disk_ok = Column(Float)
    #arc__disk_not_ok = Column(Float)
    #arc__cpu_ok = Column(Float)
    #arc__cpu_not_ok = Column(Float)
    arc__ok = Column(Float)    
    arc__nok = Column(Float)
    detail__hname = Column(Text)
    detail__hgname = Column(Text)
    detail__cpu = Column(Float)
    detail__ram = Column(Float)
    detail__disk = Column(Float)



class HostEdge(AbstractEdge):
    __tablename__ = "host_edges"


class PubSubTopicNode(AbstractNode):
    __tablename__ = "pst_nodes"

    mainstat = Column(Text)
    secondarystat = Column(Integer)
    subtitle = Column(Integer)
    arc__cpu_used = Column(Float)
    arc__cpu_unused = Column(Float)
    detail__pname = Column(Text)
    detail__uname = Column(Text)
    detail__dfreq = Column(Integer)
    detail__tsize = Column(Integer)


class PubSubTopicEdge(AbstractEdge):
    __tablename__ = "pst_edges"

    mainstat = Column(Integer)
    secondarystat = Column(Text)


class ProcessNode(AbstractNode):
    __tablename__ = "process_nodes"

    mainstat = Column(Text)
    secondarystat = Column(Text)
    arc__cpu_used = Column(Float)
    arc__cpu_unused = Column(Float)
    detail__pname = Column(Text)
    detail__uname = Column(Text)


class ProcessEdge(AbstractEdge):
    __tablename__ = "process_edges"

    mainstat = Column(Text)
    secondarystat = Column(Text)


class ClientServerNode(AbstractNode):
    __tablename__ = "cs_nodes"

    mainstat = Column(Text)
    secondarystat = Column(Text)
    arc__cpu_used = Column(Float)
    arc__cpu_unused = Column(Float)
    detail__pname = Column(Text)
    detail__uname = Column(Text)


class ClientServerEdge(AbstractEdge):
    __tablename__ = "cs_edges"

    mainstat = Column(Text)
    secondarystat = Column(Text)


if __name__ == "__main__":
    # make tests here
    with open(
        os.path.join(os.path.dirname(__file__), "topics_hosts_to_test_graph.json"),
        "r",
    ) as file:
        data = json.load(file)
