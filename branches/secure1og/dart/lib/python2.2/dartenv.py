import re

DART_ENV_TEMPLATE = """
export DART_TEST="$DART_TEST"
export DART_NODES="$DART_NODES"
export DART_NUM_NODES="$DART_NUM_NODES"
export DART_MASTER="$DART_MASTER"
export DART_GEXEC_MASTER="$DART_GEXEC_MASTER"
export DART_MY_VNN="$DART_MY_VNN"
export DART_MY_IP="$DART_MY_IP"
export DART_GPID="$DART_GPID"
export DART_COMMON_DIR="$DART_COMMON_DIR"
export DART_MY_INPUT_DIR="$DART_MY_INPUT_DIR"
export DART_MY_OUTPUT_DIR="$DART_MY_OUTPUT_DIR"
export DART_ALL_OUTPUT_DIR="$DART_ALL_OUTPUT_DIR"
"""

DART_TEST_REGEXP = re.compile("\$DART_TEST")
DART_NODES_REGEXP = re.compile("\$DART_NODES")
DART_NUM_NODES_REGEXP = re.compile("\$DART_NUM_NODES")
DART_MASTER_REGEXP = re.compile("\$DART_MASTER")
DART_GEXEC_MASTER_REGEXP = re.compile("\$DART_GEXEC_MASTER")
DART_MY_VNN_REGEXP = re.compile("\$DART_MY_VNN")
DART_MY_IP_REGEXP = re.compile("\$DART_MY_IP")
DART_GPID_REGEXP= re.compile("\$DART_GPID")
DART_COMMON_DIR_REGEXP = re.compile("\$DART_COMMON_DIR")
DART_MY_INPUT_DIR_REGEXP = re.compile("\$DART_MY_INPUT_DIR")
DART_MY_OUTPUT_DIR_REGEXP = re.compile("\$DART_MY_OUTPUT_DIR")
DART_ALL_OUTPUT_DIR_REGEXP = re.compile("\$DART_ALL_OUTPUT_DIR")

class DartEnv:
    def __init__(self, test, nodes, master_int, master_ext, my_vnn):
        import dart, time
        self.test = test
        self.nodesstr = " ".join(nodes)
        self.numnodes = "%s" % len(nodes)
        self.master = master_int
        self.gexec_master = master_ext
        self.my_vnn = "%s" % my_vnn
        self.my_ip = "%s" % nodes[my_vnn]
        self.gpid = "%s" % time.time()
        self.commondir = dart.DART_COMMON_DIR
        self.myinputdir = dart.DART_MY_INPUT_DIR
        self.myoutputdir = dart.DART_MY_OUTPUT_DIR
        self.alloutputdir = dart.DART_ALL_OUTPUT_DIR        
        
    def sub(self, data):
        subdata = data
        subdata = DART_TEST_REGEXP.sub(self.test, subdata)        
        subdata = DART_NODES_REGEXP.sub(self.nodesstr, subdata)
        subdata = DART_NUM_NODES_REGEXP.sub(self.numnodes, subdata)
        subdata = DART_MASTER_REGEXP.sub(self.master, subdata)
        subdata = DART_GEXEC_MASTER_REGEXP.sub(self.gexec_master, subdata)
        subdata = DART_MY_VNN_REGEXP.sub(self.my_vnn, subdata)
        subdata = DART_MY_IP_REGEXP.sub(self.my_ip, subdata)
        subdata = DART_GPID_REGEXP.sub(self.gpid, subdata)
        subdata = DART_COMMON_DIR_REGEXP.sub(self.commondir, subdata)
        subdata = DART_MY_INPUT_DIR_REGEXP.sub(self.myinputdir, subdata)
        subdata = DART_MY_OUTPUT_DIR_REGEXP.sub(self.myoutputdir, subdata)
        subdata = DART_ALL_OUTPUT_DIR_REGEXP.sub(self.alloutputdir, subdata)
        return subdata
