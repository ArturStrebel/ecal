import base64
from collections import defaultdict
import json


def convert_bytes_to_str(d, handle_bytes="decode"):
    """
    Recursively converts all byte objects in a nested data structure to a string representation.

    Parameters:
      d: The input data structure, which can be a dictionary, list, tuple, set, or other types.
      handle_bytes (str or callable): Determines how bytes objects are handled:
        - 'decode': Encode bytes as a base64 string (default).
        - 'clear': Replace bytes with an empty string.
        - A callable: Apply the provided function to bytes objects.

    Returns:
      The input data structure with bytes objects converted according to the `handle_bytes` parameter.
    """

    def handle_bytes_func(b):
        """
        Handles the conversion of a bytes object based on the `handle_bytes` parameter.

        Parameters:
          b: The bytes object to be handled.

        Returns:
          A string representation of the bytes object based on the `handle_bytes` parameter.

        Raises:
          ValueError: If the `handle_bytes` parameter is not recognized.
        """
        if handle_bytes == "decode":
            return base64.b64encode(b).decode("utf-8")
        elif handle_bytes == "clear":
            return ""
        elif callable(handle_bytes):
            return handle_bytes(b)
        else:
            raise ValueError(f"Invalid handle_bytes value: {handle_bytes}")

    if isinstance(d, dict):
        return {k: convert_bytes_to_str(v, handle_bytes) for k, v in d.items()}
    elif isinstance(d, list):
        return [convert_bytes_to_str(i, handle_bytes) for i in d]
    elif isinstance(d, tuple):
        return tuple(convert_bytes_to_str(i, handle_bytes) for i in d)
    elif isinstance(d, set):
        return {convert_bytes_to_str(i, handle_bytes) for i in d}
    elif isinstance(d, bytes):
        return handle_bytes_func(d)
    else:
        return d


def create_edge(topic_pub, topic_sub):
    edge = dict(
        id=topic_pub["hname"] + "_" + topic_sub["hname"],
        source=topic_pub["hname"],  # id of the corresponding source node of the edge
        target=topic_sub["hname"],  # id of the corresponding target node of the edge
        mainstat=topic_pub["tsize"] * topic_pub["dfreq"],  # bw of this connection
        secondarystat="",  # free parameter, maybe show latency, once available? (or max bandwidth)
        thickness=1,
        color="#F07D00",
    )  # d-fine orange
    return edge


def create_node(topic_pub, host_information):
    node = dict(
        id=topic_pub["hname"],
        title=topic_pub["hname"],
        mainstat=topic_pub["tsize"]
        * topic_pub[
            "dfreq"
        ],  # bandwidth of intrahost communication as sum over all publishers' write troughputs in  Byte/s (?)
        color="#1E9BD7",  # d-fine light blue
        icon="",  # only for "nice pictures" of nodes, otherwise show mainstat
        nodeRadius=50,  # scales with "importance"(currently: #Publisher, alternative: #processes/#connections ) of host minimum of 50 + (5 for every additional publisher)
        secondarystat=f"{host_information['disk_usage']*100} %",  # Disk Usage
        arc__used_ram=host_information["ram_usage"],  # RAM Usage of host in %
        arc__free_ram=round(number=(1 - host_information["ram_usage"]), ndigits=2),
    )
    return node

def create_host_graph_list_from_topics_and_hosts(topics, hosts):
    edge_dict = {}
    node_dict = {}
    host_dict = defaultdict(lambda: {"disk_usage": 0, "ram_usage": 0})
    for host in hosts:
        host_dict[host[1]] = {
            "disk_usage": round(number=(host[6] - host[7]) / host[6], ndigits=2),
            "ram_usage": round(number=(host[4] - host[5]) / host[4], ndigits=2),
        }

    pub_list = [pub for pub in topics if pub["direction"] == "publisher"]
    sub_list = [sub for sub in topics if sub["direction"] == "subscriber"]
    for pub in pub_list:
        for sub in sub_list:
            # check that pub and sub have same topic
            if pub["tname"] != sub["tname"]:
                continue
            if pub["hname"] == sub["hname"]:  # if process is within one host
                if pub["hname"] in node_dict.keys():
                    node = node_dict[pub["hname"]]
                    node["mainstat"] += pub["tsize"] * pub["dfreq"]
                else:
                    node_dict[pub["hname"]] = create_node(pub, host_dict[pub["hname"]])
                continue  # do not create an edge from a node to itself

            edgeID = pub["hname"] + "_" + sub["hname"]  # unique name for each host edge
            if edgeID in edge_dict.keys():
                edge = edge_dict[edgeID]

                edge["mainstat"] += (
                    pub["tsize"] * pub["dfreq"]
                )  # if edge already exists, update bandwidth of this edge
                edge["thickness"] = min(
                    20, edge["thickness"] + 1
                )  # make edge thicker for each connection, but cap at 20
            else:
                edge_dict[edgeID] = create_edge(pub, sub)
                if pub["hname"] not in node_dict.keys():
                    node_dict[pub["hname"]] = create_node(pub, host_dict[pub["hname"]])

                if sub["hname"] not in node_dict.keys():
                    node_dict[sub["hname"]] = create_node(sub, host_dict[sub["hname"]])
        if pub["hname"] in node_dict.keys(): # for intial delay in the testsetup
            node_dict[pub["hname"]][
                "nodeRadius"
            ] += 5  # per pub process increase size of host node
    return node_dict, edge_dict

def create_host_graph_list_from_topics_and_hosts_new(topics, host_dict):
    edge_dict = {}
    node_dict = {}

    pub_list = [pub for pub in topics if pub["direction"] == "publisher"]
    sub_list = [sub for sub in topics if sub["direction"] == "subscriber"]
    for pub in pub_list:
        for sub in sub_list:
            # check that pub and sub have same topic
            if pub["tname"] != sub["tname"]:
                continue
            if pub["hname"] == sub["hname"]:  # if process is within one host
                if pub["hname"] in node_dict.keys():
                    node = node_dict[pub["hname"]]
                    node["mainstat"] += pub["tsize"] * pub["dfreq"]
                else:
                    node_dict[pub["hname"]] = create_node(pub, host_dict[pub["hname"]])
                continue  # do not create an edge from a node to itself

            edgeID = pub["hname"] + "_" + sub["hname"]  # unique name for each host edge
            if edgeID in edge_dict.keys():
                edge = edge_dict[edgeID]

                edge["mainstat"] += (
                    pub["tsize"] * pub["dfreq"]
                )  # if edge already exists, update bandwidth of this edge
                edge["thickness"] = min(
                    20, edge["thickness"] + 1
                )  # make edge thicker for each connection, but cap at 20
            else:
                edge_dict[edgeID] = create_edge(pub, sub)
                if pub["hname"] not in node_dict.keys():
                    node_dict[pub["hname"]] = create_node(pub, host_dict[pub["hname"]])

                if sub["hname"] not in node_dict.keys():
                    node_dict[sub["hname"]] = create_node(sub, host_dict[sub["hname"]])
        if pub["hname"] in node_dict.keys(): # for intial delay in the testsetup
            node_dict[pub["hname"]][
                "nodeRadius"
            ] += 5  # per pub process increase size of host node
    return node_dict, edge_dict

def export_graph_structure(nodes, edges, path):
    graph = {
        "nodes": [node["id"] for node in nodes],
        "edges": [(edge["source"], edge["target"]) for edge in edges],
    }
    with open(path, "w") as json_file:
        json.dump(graph, json_file)
