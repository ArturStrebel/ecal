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

import base64
import sys
import time
import sqlite3
import random
import ecal.core.core as ecal_core


def convert_bytes_to_str(d, handle_bytes='decode'):
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
    if handle_bytes == 'decode':
      return base64.b64encode(b).decode('utf-8')
    elif handle_bytes == 'clear':
      return ''
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

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.getversion(), ecal_core.getdate()))
  
  # initialize eCAL API
  ecal_core.initialize(sys.argv, "monitoring")
  
  # initialize eCAL monitoring API
  ecal_core.mon_initialize()
  time.sleep(2)
  
  # SQLite
  conn = sqlite3.connect('ecal_monitoring.db')
  cursor = conn.cursor()
  
  def create_process_table(table_name):
    cursor.execute('DROP TABLE IF EXISTS ' + table_name)
    cursor.execute('''
      CREATE TABLE ''' + table_name + '''(
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
    ''')
    
  def create_service_table(table_name):
    cursor.execute('DROP TABLE IF EXISTS ' + table_name)
    cursor.execute('''
      CREATE TABLE ''' + table_name + '''( 
      time_stamp TEXT,
      pid INTEGER,
      rclock INTEGER,
      hname TEXT,
      pname TEXT,
      uname TEXT,
      sname TEXT)
    ''')
      
  def create_topic_table(table_name):
    cursor.execute('DROP TABLE IF EXISTS ' + table_name)
    cursor.execute('''
      CREATE TABLE ''' + table_name + '''(
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
      latency_min INTEGER,
      latency_avg INTEGER,
      latency_max INTEGER)
    ''')
      
  # Create tables 
  create_process_table('processes')
  create_process_table('current_processes')
  create_process_table('previous_processes')
  create_process_table('dropped_processes')
  
  create_service_table('services')
  create_service_table('current_services')
  create_service_table('previous_services')
  
  create_topic_table('topics')
  create_topic_table('current_topics')
  create_topic_table('previous_topics')
  
  # host graph tables (dont change any names in these two tables!)
  cursor.execute('DROP TABLE IF EXISTS edges')
  cursor.execute('DROP TABLE IF EXISTS nodes')
  cursor.execute('''
    CREATE TABLE edges (
    id TEXT PRIMARY KEY,
    source TEXT,
    target TEXT,
    mainstat TEXT,
    secondarystat TEXT,
    thickness INTEGER, 
    color TEXT)
  ''')
  
  cursor.execute('''
    CREATE TABLE nodes (
    id TEXT PRIMARY KEY,
    title TEXT,
    mainstat TEXT,
    color TEXT, 
    icon TEXT,
    nodeRadius INTEGER)
  ''')
  
  def insert_process(process, table_name):
    cursor.execute('''
      INSERT INTO ''' + table_name + ''' (
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
      ''', (process['pid'], process['rclock'], process['hname'], process['pname'], process['uname'], process['pparam'], 
            process['pmemory'], process['pcpu'], process['usrptime'], process['datawrite'], process['state_severity'], process['state_severity_level'], 
            process['state_info'], process['tsync_state'], process['tsync_mod_name'], process['component_init_state'], process['component_init_info']))
  
  def insert_service(service, table_name):
    cursor.execute('''
      INSERT INTO ''' + table_name + ''' (
      time_stamp,
      pid,
      rclock,
      hname,
      pname,
      uname,
      sname)
      VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?)
      ''', (service['pid'], service['rclock'], service['hname'], service['pname'], service['uname'], service['sname']))
  
  def insert_topic(topic, table_name):
    cursor.execute('''
      INSERT INTO ''' + table_name + ''' (
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
      latency_min,
      latency_avg,
      latency_max)
      VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )
      ''', (topic['pid'], topic['rclock'], topic['hname'], topic['pname'], topic['uname'], topic['tid'],
            topic['tname'], topic['direction'], topic['ttype'], topic['tdesc'], topic['tsize'], 
            topic['dclock'], topic['dfreq'], random.randint(1,20), random.randint(21,50), random.randint(51,120)))
  
  def insert_edge(edge, table_name) :
    cursor.execute('''INSERT INTO ''' + table_name +'''(id, source, target, mainstat, secondarystat, thickness, color) 
                 VALUES (?, ?, ?, ?, ?, ?, ?)''', 
                 (edge['id'], edge['source'], edge['target'], edge['mainstat'], 
                 edge['secondarystat'], edge['thickness'], edge['color']))
  
  def insert_node(node, table_name) :
    cursor.execute('''INSERT INTO ''' + table_name + '''(id, title, mainstat, color, icon, nodeRadius)
                 VALUES (?, ?, ?, ?, ?, ?)''', 
                 (node['id'], node['title'], node['mainstat'], node['color'], 
                 node['icon'], node['nodeRadius']))
    
  # Test graph (comment out live updates in while loop to show this)
  cursor.execute('''INSERT INTO edges (id, source, target, mainstat, secondarystat, thickness, color) 
                 VALUES ('e1', 'Rear View', 'HPC Controller' , '3.14 Mbit/s', 'Camera Stream', 9, '#F07D00')''')
  cursor.execute('''INSERT INTO edges (id, source, target, mainstat, secondarystat, thickness, color) 
                 VALUES ('e2', 'HPC Controller', 'Info', '749 Kbit/s', 'System metrics', 5, '#F07D00')''')
  cursor.execute('''INSERT INTO edges (id, source, target, mainstat, secondarystat, thickness, color) 
                 VALUES ('e3', 'Radio', 'Info', '128 Kbit/s', 'Music', 2, '#F07D00')''')
  cursor.execute('''INSERT INTO edges (id, source, target, mainstat, secondarystat, thickness, color) 
                VALUES ('e4', 'Phone', 'Info', '1.50 Mbit/s', 'Navigation', 7, '#F07D00')''')
  cursor.execute('''INSERT INTO edges (id, source, target, mainstat, secondarystat, thickness, color) 
                VALUES ('e5', 'Rear View', 'Info', '3.14 Mbit/s', 'Rear view camera', 9, '#F07D00')''')
  cursor.execute('''INSERT INTO edges (id, source, target, mainstat, secondarystat, thickness, color) 
                VALUES ('e6', 'Sensor', 'HPC Controller', '941 Bit/s', 'Health parameters', 2, '#F07D00')''') 
   
  cursor.execute('''INSERT INTO nodes (id, title, mainstat, color, icon, nodeRadius) 
                 VALUES ('Info', 'Infotainment system', '0 Bit/s', '#1E9BD7', 'home-alt', 60)''')
  cursor.execute('''INSERT INTO nodes (id, title, mainstat, color, icon, nodeRadius) 
                 VALUES ('Phone', 'Smartphone', '300.65 Mit/s', '#1E9BD7', 'mobile-android', 50)''')
  cursor.execute('''INSERT INTO nodes (id, title, mainstat, color, icon, nodeRadius) 
                 VALUES ('Radio', 'Radio', '0 Bit/s', '#1E9BD7', 'rss', 40)''')
  cursor.execute('''INSERT INTO nodes (id, title, mainstat, color, icon, nodeRadius) 
                 VALUES ('Rear View', 'Rear View', '42 Bit/s', '#1E9BD7', 'camera', 40)''')
  cursor.execute('''INSERT INTO nodes (id, title, mainstat, color, icon, nodeRadius) 
                 VALUES ('HPC Controller', 'HPC Controller','140 Mbit/s', '#1E9BD7', 'calculator-alt', 50)''')
  cursor.execute('''INSERT INTO nodes (id, title, mainstat, color, icon, nodeRadius) 
                 VALUES ('Sensor', 'Multiple Sensors', '0 Bit/s', '#1E9BD7', 'exclamation', 40)''')
  
  def create_edge(pub, sub, edge_list) :
    edge = dict(id = pub['hname'] + '_' + sub['hname'],
              source = pub['pid'], # unique id of pub
              target = sub['pid'], # unique id of sub
              mainstat = pub['tsize'], # package size of this connection
              secondarystat = '', # free parameter, maybe show latency, once available? (or max bandwidth)
              thickness = 1,
              color = '#F07D00') # d-fine orange
    edge_list.append(edge)
    insert_edge(edge, 'edges') # add edge to database
    
  def create_node(process, node_list) :
    node = dict(id = process['hname'],
                  title = process['hname'], 
                  mainstat = process['tsize'], # bandwidth of intrahost communication
                  color = '#1E9BD7', # d-fine light blue
                  icon = '', # only for "nice pictures" of nodes, otherwise show mainstat 
                  nodeRadius = 50) # could scale with "importance" of this host
    node_list.append(node)
    insert_node(node, 'nodes') # add node to database
  
  def update_intrahost_process(pub, node_list) :
    found_host = 0
    for node in node_list :
      if node['id'] == pub['hname'] : # if the host already exists update bandwidth
        node['mainstat'] += pub['tsize']
        found_host = 1
    if found_host == 0 : # else create node
      create_node(pub, node_list)
   
  def create_host_graph_list(edge_list, node_list):
    ecal_mon = ecal_core.mon_monitoring()
    topics = ecal_mon[1]['topics']
    #insert fake topics here
    fakepub = dict(rclock = 0, # not important for fake
                hname = 'fakehost1',
                pid = -1, # should be a unique number 
                pname = '', # not important for fake
                uname = '', # not important for fake
                tid = -1, # not important for fake
                tname = 'faketopic1',
                direction = 'publisher', 
                ttype = '', # not important for fake
                tdesc = '', # not important for fake
                tsize = 13, # used to compute bandwidth
                dclock = 0, # not important for fake
                dfreq = 2000) # used to compute bandwidth
    fakesub = dict(rclock = 0, # not important for fake
                hname = 'fakehost1',
                pid = -1, # should be a unique number 
                pname = '', # not important for fake
                uname = '', # not important for fake
                tid = -1, # not important for fake
                tname = 'faketopic1',
                direction = 'subscriber', 
                ttype = '', # not important for fake
                tdesc = '', # not important for fake
                tsize = 13, # used to compute bandwidth
                dclock = 0, # not important for fake
                dfreq = 2000) # used to compute bandwidth

    topics.append(fakepub)
    topics.append(fakesub)
    
    pub_list = [pub for pub in topics if pub['direction'] == 'publisher']
    sub_list = [sub for sub in topics if sub['direction'] == 'subscriber']
    for pub in pub_list:
      for sub in sub_list:
        
        # check that pub and sub have same topic
        if pub['tname'] != sub['tname'] :
          continue
                
        if pub['hname'] == sub['hname'] : # if process is within one host
          update_intrahost_process(pub, node_list)
          continue # do not create an edge from a node to itself
          
        edgeID = pub['hname'] + '_' + sub['hname'] # unique name for each host edge
        found_edge = 0
        for edge in edge_list:
          if edge['id'] == edgeID:
            edge['mainstat'] += pub['tsize'] # if edge already exists, update bandwidth of this edge
            edge['thickness'] = min( 20, edge['thickness'] + 1) # make edge thicker for each connection, but cap at 20
            found_edge = 1

        if found_edge == 0 : # found a new edge
          create_edge(pub, sub, edge_list)
          
          # either pub or sub (or both) is a new node, which needs to be added to node list          
          found_pub = 0
          for node in node_list :
            if node['id'] == pub['pid'] :
              found_pub = 1
          if found_pub == 0 :
            create_node(pub, node_list)
              
          found_sub = 0
          for node in node_list :
            if node['id'] == sub['pid'] :
              found_sub = 1
          if found_sub == 0 :
            create_node(sub, node_list)

  # print eCAL entities
  while ecal_core.ok():
    # convert 'bytes' type elements of the the monitoring dictionary
    monitoring_d = convert_bytes_to_str(ecal_core.mon_monitoring(), handle_bytes = 'decode')
    monitoring = monitoring_d[1]
    print(monitoring)
    
    # save previous process state
    cursor.execute('DELETE FROM previous_processes')
    cursor.execute('DELETE FROM previous_services')
    cursor.execute('DELETE FROM previous_topics')
    
    cursor.execute('INSERT INTO previous_processes SELECT * FROM current_processes')
    cursor.execute('INSERT INTO previous_services SELECT * FROM current_services')
    cursor.execute('INSERT INTO previous_topics SELECT * FROM current_topics')
    
    # update current process state
    cursor.execute('DELETE FROM current_processes')
    cursor.execute('DELETE FROM current_services')
    cursor.execute('DELETE FROM current_topics')
    
    for process in monitoring['processes']:
      insert_process(process, 'processes')
      insert_process(process, 'current_processes')

    for service in monitoring['services']:
      insert_service(service, 'services')
      insert_service(service, 'current_services')

    for topic in monitoring['topics']:
      insert_topic(topic, 'topics')
      insert_topic(topic, 'current_topics')
    
    # check for dropped processes and add them to DB  
    cursor.execute('''INSERT INTO dropped_processes 
                  SELECT previous_processes.* FROM previous_processes 
                  LEFT JOIN current_processes ON previous_processes.pid = current_processes.pid
                  WHERE current_processes.pid IS NULL''')
    
    # update host traffic table
    cursor.execute('DELETE FROM edges')
    cursor.execute('DELETE FROM nodes')
    edge_list = []
    node_list = []
    create_host_graph_list(edge_list, node_list)
      
    conn.commit()
    time.sleep(1)
          
  # finalize eCAL monitoring API
  ecal_core.mon_finalize()
  
  # finalize eCAL API
  ecal_core.finalize()
 
  conn.close()
  
if __name__ == "__main__":
  main()  
