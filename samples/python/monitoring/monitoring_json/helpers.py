import base64
import json

THRESHOLD = 0.8

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


def create_edge(
    id,
    source,
    target,
    mainstat,
    secondarystat,
    details={},
    thickness=1,
    highlighted=False,
    color="#EFEEEB",
    stroke_dasharray="",
):
    edge = {
        "id": id,
        "source": source,
        "target": target,
        "mainstat": mainstat,
        "secondarystat": secondarystat,
        "thickness": thickness,
        "highlighted": highlighted,
        "color": color,
        "strokeDasharray": stroke_dasharray,
    }

    for detail_key, detail_value in details.items():
        edge[detail_key] = detail_value

    return edge


def create_node(
    id,
    title,
    subtitle,
    mainstat,
    secondarystat,
    arcs={},
    details={},
    color="#EFEEEB",
    icon="",
    node_radius=60,
    highlighted="",
):
    node = {
        "id": id,
        "title": title,
        "subtitle": subtitle,
        "mainstat": mainstat,
        "secondarystat": secondarystat,
        "color": color,
        "icon": icon,
        "nodeRadius": node_radius,
        "highlighted": highlighted,
    }

    for arc_key, arc_value in arcs.items():
        node[arc_key] = arc_value

    for detail_key, detail_value in details.items():
        node[detail_key] = detail_value

    return node


def get_arc_cpu(process_performances, hname, pid):
    if process_performances:
        cpu_load = process_performances[hname][pid]["cpu_load"]
        if cpu_load == -1:
            return {}
        else:
            cpu_load = round(number=cpu_load / 100, ndigits=5)
        arcs = {"arc__cpu_used": cpu_load, "arc__cpu_unused": (1 - cpu_load)}
    else:
        arcs = {"arc__cpu_used": None, "arc__cpu_unused": None}
    return arcs


def create_host_graph(topics, host_dict):
    edge_dict = {}
    node_dict = {}

    unique_hosts = {t["hname"]: t for t in topics}
    for host in unique_hosts.values():
        print("host    ", host_dict)
        host_data = host_dict[host["hname"]]
        arcs = {
            #"arc__used_ram": host_dict[host["hname"]]["ram_usage"],
            #"arc__free_ram": round(
            #    number=(1 - host_dict[host["hname"]]["ram_usage"]), ndigits=2
            #),
            "arc__ram_ok": 0.33 if host_data["ram_usage"] <= THRESHOLD else 0,
            "arc__ram_not_ok": 0.33 if host_data["ram_usage"] > THRESHOLD else 0,
            "arc__disk_ok": 0.33 if host_data["disk_usage"] <= THRESHOLD else 0,
            "arc__disk_not_ok": 0.33 if host_data["disk_usage"] > THRESHOLD else 0,
            "arc__cpu_ok": 0.34 if host_data["cpu_load"] <= THRESHOLD else 0,
            "arc__cpu_not_ok": 0.34 if host_data["cpu_load"] > THRESHOLD else 0,
        }
        details = {
            "detail__hname": host["hname"],
            "detail__hgname": host["hgname"],
        }
        node_dict[host["hname"]] = create_node(
            id=host["hname"],
            title=host["hname"],
            subtitle="",
            mainstat=0,
            secondarystat=host_dict[host["hname"]]["disk_usage"] * 100,
            arcs=arcs,
            details=details,
        )
    pub_list = [pub for pub in topics if pub["direction"] == "publisher"]
    sub_list = [sub for sub in topics if sub["direction"] == "subscriber"]
    for pub in pub_list:
        for sub in sub_list:
            # check that pub and sub have same topic
            if pub["tname"] != sub["tname"]:
                continue
            if pub["hname"] == sub["hname"]:  # if process is within one host
                node = node_dict[pub["hname"]]
                node["mainstat"] += pub["tsize"] * pub["dfreq"]
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
                edge_dict[edgeID] = create_edge(
                    id=edgeID,
                    source=pub["hname"],
                    target=sub["hname"],
                    mainstat=pub["tsize"] * pub["dfreq"],
                    secondarystat="",
                    thickness=1,
                    color="#EFEEEB",
                )

        node_dict[pub["hname"]][
            "nodeRadius"
        ] += 5  # per pub process increase size of host node

    return node_dict, edge_dict


def create_process_graph(topics, process_performances):
    edge_dict = {}
    node_dict = {}

    pubs = []
    subs = []
    for t in topics:
        details = {
                "detail__pname": t["pname"],
                "detail__uname": t["uname"],
            }
        if t["direction"] == "publisher":
            pubs.append(t)
        if t["direction"] == "subscriber":
            subs.append(t)
        node_id = f"{t['hname']}-{t['pid']}"
        if node_id not in node_dict.keys():
            arcs = get_arc_cpu(process_performances=process_performances, hname=t["hname"], pid=t["pid"])
            node_dict[node_id] = create_node(
                id=node_id,
                title=t["hname"],
                subtitle="",
                mainstat=t["pid"],
                secondarystat=t["tname"],
                arcs=arcs,
                details=details
            )
        else:
            node_dict[node_id]["mainstat"] = (
                node_dict[node_id]["mainstat"] + f" {t['tname']}"
            )

    for pub in pubs:
        pub_proc_id = f"{pub['hname']}-{pub['pid']}"
        for sub in subs:
            # check that pub and sub have same topic
            if pub["tname"] != sub["tname"]:
                continue
            sub_proc_id = f"{sub['hname']}-{sub['pid']}"
            if pub_proc_id == sub_proc_id:
                continue  

            edge_id = f"{pub_proc_id }-{sub_proc_id}"
            if edge_id in edge_dict.keys():
                edge_dict[edge_id][
                    "thickness"
                ] += 1  # inc thickness per pub - sub connection
                edge_dict[edge_id]["secondarystat"] = (
                    edge_dict[edge_id]["secondarystat"] + pub["tname"]
                )
            else:
                edge_dict[edge_id] = create_edge(
                    id=edge_id,
                    source=pub_proc_id,
                    target=sub_proc_id,
                    mainstat="",
                    secondarystat=pub["tname"],
                )

    return node_dict, edge_dict


def create_pub_sub_topic_graph(topics, process_performances):
    edge_dict = {}
    node_dict = {}

    for t in topics:
        details = {
                "detail__pname": t["pname"],
                "detail__uname": t["uname"],
                "detail__dfreq": t["dfreq"],
                "detail__tsize": t["tsize"],
            }
        if t["direction"] == "publisher":
            arcs = get_arc_cpu(process_performances=process_performances, hname=t["hname"], pid=t["pid"])
            node_id = t["tid"]
            node_dict[node_id] = create_node(
                id=node_id,
                title=t["hname"],
                subtitle="",
                mainstat=t["pid"],
                secondarystat=t["tname"],
                arcs=arcs,
                details=details
            )

            edge_id = f"{node_id}-{t['tname']}"
            edge_dict[edge_id] = create_edge(
                id=edge_id,
                source=node_id,
                target=t["tname"],
                mainstat=t["message_drops"],
                secondarystat="",
            )
        if t["direction"] == "subscriber":
            arcs = get_arc_cpu(process_performances=process_performances, hname=t["hname"], pid=t["pid"])
            node_id = t["tid"]
            node_dict[node_id] = create_node(
                id=node_id,
                title=t["hname"],
                subtitle="",
                mainstat=t["pid"],
                secondarystat=t["tname"],
                arcs=arcs,
                details=details
            )
            edge_id = f"{t['tname']}-{node_id}"
            edge_dict[edge_id] = create_edge(
                id=edge_id,
                source=t["tname"],
                target=node_id,
                mainstat=t["message_drops"],
                secondarystat="",
            )

        node_id = t["tname"]  # for topic
        node_dict[node_id] = create_node(
            id=node_id,
            title=t["tname"],
            subtitle="",
            mainstat=None,
            secondarystat=t["tname"],
            highlighted="true",
            icon="comment-alt",
        )

    return node_dict, edge_dict


def create_client_server_graph(clients, services, process_performances):
    edge_dict = {}
    node_dict = {}
    for client in clients:
        client_id = client["sid"]
        details = {
                "detail__pname": client["pname"],
                "detail__uname": client["uname"],
            }
        arcs = get_arc_cpu(process_performances=process_performances, hname=client["hname"], pid=client["pid"])
        node_dict[client_id] = create_node(
            id=client_id,
            title=client["hname"],
            subtitle="",
            mainstat=client["pid"],
            secondarystat=client["sname"],
            arcs=arcs,
            details=details
        )
    for service in services:
        details = {
                "detail__pname": service["pname"],
                "detail__uname": service["uname"],
            }
        service_id = service["sid"]
        arcs = get_arc_cpu(process_performances=process_performances, hname=service["hname"], pid=service["pid"])
        node_dict[service_id] = create_node(
            id=service_id,
            title=service["hname"],
            subtitle="",
            mainstat=service["pid"],
            secondarystat=service["sname"],
            arcs=arcs,
            details=details
        )

    for client in clients:
        for service in services:
            if client["sname"] != service["sname"]:
                continue
            edge_id = client["sid"] + "_" + service["sid"]
            edge_dict[edge_id] = create_edge(
                id=edge_id,
                source=client["sid"],
                target=service["sid"],
                mainstat="",
                secondarystat=service["sname"],
            )
    return node_dict, edge_dict


def export_graph_structure(nodes, edges, path):
    graph = {
        "nodes": [node["id"] for node in nodes],
        "edges": [(edge["source"], edge["target"]) for edge in edges],
    }
    with open(path, "w") as json_file:
        json.dump(graph, json_file)
