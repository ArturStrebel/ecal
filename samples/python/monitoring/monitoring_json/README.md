# eCAL Dashboard

The eCAL Dashboard provides functionality for monitoring the state of the eCAL middleware. The dashboard offers insights into:

- The status of the system's hosts, including their RAM, Disk, and CPU.
- The communication topology present in the system, including:
  - Interactions between publishers and subscribers of the topics,
  - Connections between clients and services,
  - Summary data at the process level,
  - Aggregated information at the host level.
- Performance metrics for both publishers and subscribers, such as write and read performance, as well as message drops.

## Architecture

The application consists of a Python script that polls eCAL's Monitoring API at predefined intervals. It subscribes to the "machine_state_[HOSTNAME]"-named topics, to which eCAL's Machine Monitoring Agents (MMAs) publish information regarding individual hosts (CPU, RAM, Disk, etc.). The collected data is then inserted into a SQLite database, which serves as the data source for the Grafana dashboards.

## Usage

To monitor the eCAL middleware using the eCAL Dashboard, follow these steps:

1. Start the MMAs on the system's hosts using the script ecal_mma.exe (Windows), which can be found in the `\bin` folder of a host's eCAL directory.
2. Before starting the script, ensure that the required packages are installed. These can be found in the requirements.txt file. You can install the requirements using the command:
```bash 
pip install -r requirements.txt
```
3. Additionally, install a Python Wheel for eCAL6, available from the [eCAL repository](https://github.com/eclipse-ecal/ecal). To facilitate the usage of the script prior to an official eCAL6 release, the wheel used for development is provided and can be installed using pip.
4. Execute the script with the following command:
```python 
python monitoring_json.py --interval 1
``` 
The `--interval` flag specifies the polling interval; in this case, the Monitoring API is polled every second. Appending `--verbose` will provide verbose output from the script. The monitoring script creates a database named `ecal_monitoring.db` in the `\db` directory, which is then accessed by the Grafana server.

5. Start the Grafana server instance using the `docker-compose.yaml` file by executing the command (in detached mode): 

```bash 
docker-compose up -d
```

6. Access the Grafana dashboard in your browser via ```localhost:3000```. To login use the default user ```admin``` with the default password ```admin```.


