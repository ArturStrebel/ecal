import base64

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

def create_edge(pub, sub): 
  edge = dict(id = pub['hname'] + '_' + sub['hname'],
              source = pub['pid'], # unique id of pub
              target = sub['pid'], # unique id of sub
              mainstat = pub['tsize'], # package size of this connection
              secondarystat = '', # free parameter, maybe show latency, once available? (or max bandwidth)
              thickness = 1,
              color = '#F07D00') # d-fine orange
  return edge
    
def create_node(process):
  node = dict(id = process['hname'],
                title = process['hname'], 
                mainstat = process['tsize'], # bandwidth of intrahost communication
                color = '#1E9BD7', # d-fine light blue
                icon = '', # only for "nice pictures" of nodes, otherwise show mainstat 
                nodeRadius = 50) # could scale with "importance" of this host
  return  node

def update_or_create_intrahost_process(pub, node_list) :
  for node in node_list :
    if node['id'] == pub['hname'] : # if the host already exists update bandwidth
      node['mainstat'] += pub['tsize']
      return None
  return create_node(pub)
   
def create_host_graph_list_from_topics(topics):
  edge_list = []
  node_list = []
  #insert fake topics here
  fakepub = dict(rclock = 1, # not important for fake
              hname = 'fakehost1',
              pid = 12345678, # should be a unique number 
              pname = 'a', # not important for fake
              uname = 'b', # not important for fake
              tid = 1, # not important for fake
              tname = 'faketopic1',
              direction = 'publisher', 
              ttype = 'c', # not important for fake
              tdesc = 'd', # not important for fake
              tsize = 13, # used to compute bandwidth
              dclock = 1, # not important for fake
              dfreq = 2000) # used to compute bandwidth
  fakesub = dict(rclock = 1, # not important for fake
              hname = 'fakehost1',
              pid = 2345678, # should be a unique number 
              pname = 'aa', # not important for fake
              uname = 'bb', # not important for fake
              tid = 1, # not important for fake
              tname = 'faketopic1',
              direction = 'subscriber', 
              ttype = 'cc', # not important for fake
              tdesc = 'dd', # not important for fake
              tsize = 13, # used to compute bandwidth
              dclock = 1, # not important for fake
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
        node = update_or_create_intrahost_process(pub, node_list)
        if node is not None:
          node_list.append(node)
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
          node_list.append(create_node(pub))
            
        found_sub = 0
        for node in node_list :
          if node['id'] == sub['pid'] :
            found_sub = 1
        if found_sub == 0 :
          node_list.append(create_node(sub))
  return node_list, edge_list
