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
    cursor.execute('''
      CREATE TABLE IF NOT EXISTS ''' + table_name + '''(
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
    cursor.execute('''
      CREATE TABLE IF NOT EXISTS ''' + table_name + '''( 
      time_stamp TEXT,
      pid INTEGER,
      rclock INTEGER,
      hname TEXT,
      pname TEXT,
      uname TEXT,
      sname TEXT)
    ''')
      
  def create_topic_table(table_name):
    cursor.execute('''
      CREATE TABLE IF NOT EXISTS ''' + table_name + '''(
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
      dfreq INTEGER)
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
  
  # host graph tables
  cursor.execute('''
    CREATE TABLE IF NOT EXISTS edges (
    id TEXT PRIMARY KEY,
    source TEXT,
    target TEXT,
    mainstat REAL,
    thickness INTEGER, 
    color TEXT)
  ''')
  
  cursor.execute('''
    CREATE TABLE IF NOT EXISTS nodes (
    id TEXT PRIMARY KEY,
    mainstat INTEGER,
    nodeRadius INTEGER)
  ''')
  
  # Clear db for a new start
  cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
  tables = cursor.fetchall()
  for table_name in tables:
    cursor.execute(f'DELETE FROM {table_name[0]};')
  conn.commit()
  
  def insert_process(process, processes):
    cursor.execute('''
      INSERT INTO ''' + processes + ''' (
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
  
  def insert_service(service, services):
    cursor.execute('''
      INSERT INTO ''' + services + ''' (
      time_stamp,
      pid,
      rclock,
      hname,
      pname,
      uname,
      sname)
      VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?)
      ''', (service['pid'], service['rclock'], service['hname'], service['pname'], service['uname'], service['sname']))
  
  def insert_topic(topic, topics):
    cursor.execute('''
      INSERT INTO ''' + topics + ''' (
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
      dfreq)
      VALUES (datetime('now', 'localtime'), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
      ''', (topic['pid'], topic['rclock'], topic['hname'], topic['pname'], topic['uname'], topic['tid'],
            topic['tname'], topic['direction'], topic['ttype'], topic['tdesc'], topic['tsize'], topic['dclock'], topic['dfreq']))
  
  # Test graph
  cursor.execute('''INSERT INTO edges (id, source, target) VALUES ('e1', 'host1', 'host2')''')
  cursor.execute('''INSERT INTO nodes (id, mainstat) VALUES ('host1', 10)''')
  cursor.execute('''INSERT INTO nodes (id, mainstat) VALUES ('host2', 100)''')
  
  def create_edge_list():
    ecal_mon = ecal_core.mon_monitoring()
    topics = ecal_mon[1]['topics']
    for pub in topics:
      for sub in topics:
        edgeID = pub['hname'] + '_' + sub['hname']
        
  
  # print eCAL entities
  while ecal_core.ok():
    # convert 'bytes' type elements of the the monitoring dictionary
    monitoring_d = convert_bytes_to_str(ecal_core.mon_monitoring(), handle_bytes = 'decode')
    monitoring = monitoring_d[1]
    
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
      
    conn.commit()
    time.sleep(5)
          
  # finalize eCAL monitoring API
  ecal_core.mon_finalize()
  
  # finalize eCAL API
  ecal_core.finalize()
 
  conn.close()
  
if __name__ == "__main__":
  main()  
