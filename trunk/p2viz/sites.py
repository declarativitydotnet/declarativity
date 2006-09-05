#!/usr/local/bin/python
import xmlrpclib
import sys
import os
import getopt

def print_usage():
    print "Usage: sites.py [<slice_name>]"

def parse_cmdline(argv):
    shortopts = ""
    flags = {}
    opts, args = getopt.getopt(argv[1:], shortopts)
    return flags, args

def getNodes(slice):
  auth = {}
  auth['AuthMethod'] = "anonymous"

  sliceNodes = {}
  if slice:
    nodes = server.AnonSliceNodesList(auth, slice)
    for n in nodes:
      sliceNodes[n] = True

  # get the all the nodes, regardless of current boot state
  nodes = {}
  node_fields = ['node_id', 'hostname', 'model', 'version', 'boot_state', 'ip', 'mac'] 
  all_nodes = server.AnonAdmGetNodes(auth, [], node_fields)
  for n in all_nodes:
    nodes[n['node_id']] = n 

  all_sites      = server.AnonAdmGetSites(auth)
  all_site_nodes = server.AnonAdmGetSiteNodes(auth)
  for s in all_sites:
    siteid = str(s['site_id'])
    if all_site_nodes.has_key(siteid):
      site_nodes = all_site_nodes[siteid] 
      for n in site_nodes:
        node = nodes[n]
        if sliceNodes and not sliceNodes.has_key(node['hostname']): continue
        if not s['longitude'] or not s['latitude']:
          print "NO LONGITUDE OR LATITUDE FOR NODE: ", node['hostname']
          continue
        node_info.append([node['node_id'], node['hostname'], node['model'], node['version'], \
                          node['boot_state'], node['ip'], node['mac'], \
                          s['site_id'], s['name'], s['longitude'], s['latitude'], s['url']])

if __name__ == "__main__":
  global server, node_info
  try:
    flags, args = parse_cmdline(sys.argv)
  except:
    print "EXCEPTION"
    print_usage()
    sys.exit(1)

  result = open("sites.out", 'w')
  slice = None
  if len(args) > 0:
    slice = args[0]

  server = xmlrpclib.Server('https://www.planet-lab.org/PLCAPI/')

  node_info = []
  getNodes(slice)

  for node in node_info:
      line = ""
      for i in node[:-1]: 
        try:
          value = str(i).strip()
        except:
          value = "unknown"
        if not value: value = "unknown"
        line += "%s,," % value
      try:
        value = str(node[-1]).strip()
      except:
        value = "unknown"
      if not value: value = "unknown"
      line += "%s" % value
      print >> result, line
