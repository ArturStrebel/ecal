from DatabaseHandler import DatabaseHandler
from google.protobuf.json_format import MessageToDict

def host_monitor_callback(topic_name, msg, time):
  db_handler_callback = DatabaseHandler()

  pids = db_handler_callback.execute_command('SELECT DISTINCT PID from processes').fetchall() 
  pids = {id[0] for id in pids}
    
  message_as_dict = MessageToDict(message=msg)
  hname = topic_name.split('_')[2]

  db_handler_callback.insert_host(hname, message_as_dict, 'hosts')

  processes = message_as_dict['process']
  processes = [p for p in processes if p.get('id', -1) in pids]
  for process in processes:
    db_handler_callback.insert_process_performance(hname, process, 'process_performance')

  db_handler_callback.commit()
  db_handler_callback.close()