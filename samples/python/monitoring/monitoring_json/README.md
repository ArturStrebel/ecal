# eCAL Dashboard

The eCAL Dashboard provides functionality to monitor the state of the eCAL middleware. The dashboard gives insights on:
- The state of the system's hosts, such as their RAM, Disk, and CPU.
- The topology of the communication present in the system, with regards to the communication:
    - between the publishers and subscribers of the topics,
    - the clients and the services,
    - summarized on a process level,
    - and aggregated on a host level.
- Information on the publisher and subscriber performance, such as write& read performance, and message drops. 


## Architecture

The application consists of a Python script. The script polls eCAL's Monitoring API in predifined intervals and provides subscribers to the "machine_state_[HOSTNAME]"-named topics to which eCAL's Machine Monitoring Agents (MMAs) publish the information regarding the individual hosts (CPU, RAM, Disk, ...). Next, the collected data is inserted into a SQLite database, which then serves as the data source for the Grafana dashboards. 

## Usage

The following steps have to be done to be able to montior the eCAL middleware using the eCAL Dashboard:
- Set up a Grafana server instance and import the Grafana dashboards that are provided as JSON files in the "grafana_dashboards" folder onto this server.
- Configure the SQLite data source for the Grafana dashboards. The monitoring script creates a database ```ecal_monitoring.db``` in the same folder as the script.
- Start the MMAs on the systems hosts, via the script ```ecal_mma.exe``` (Windows), which can be found in the ```\bin``` folder of a hosts eCAL folder.
- Before starting the script, ensure that the requirements are installed, which can be found in the ```requirements.txt``` file. In addition, install a Python Wheel for eCAL6, which can be obtained from the [eCAL repository](https://github.com/eclipse-ecal/ecal).
- Execute the script via the following command ```python monitoring_json.py --interval 1```. With the interval flag, the polling interval can be specified. In this case, every second the Monitoring API is polled. Furthermore, appending ```--verbose``` allows for verbose outputs of the script.