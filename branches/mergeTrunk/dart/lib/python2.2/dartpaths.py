import os

# NOTE: specific paths for DART_INSTALL_DIR, Remote Setup

# Remote Setup
DART_RPMS         = [ "/proj/P2/rpms/authd-0.2.1-1.i386.rpm",
                      "/proj/P2/rpms/gexec-0.3.5-1.i386.rpm",
                      "/proj/P2/rpms/libe-0.2.2-1.i386.rpm",
                      "/proj/P2/rpms/pcp-0.3.2-1.i386.rpm",
                      "/proj/P2/rpms/gexec-pcp-fix-1-1.i386.rpm" ]
DART_KEYS_TARFILE   = "/proj/P2/tarfiles/authkeys.tar.gz"

DART_SETUP_SCRIPT   = "/proj/P2/setup"

# Local
DART_INSTALL_DIR    = "%s/dart" % os.getenv("HOME")
DART_ENV_TEMPLATE   = "%s/etc/env.dart" % DART_INSTALL_DIR

# Remote
DART_COMMON_DIR     = "/tmp"
DART_MY_INPUT_DIR   = "/tmp/in"
DART_MY_OUTPUT_DIR  = "/tmp/out"
DART_ALL_OUTPUT_DIR = "/tmp/allout"

# Remote DART reserved directory
DART_RESERVED_DIR     = "/tmp/dart"                           # Master/Slaves
DART_PROFILE          = "%s/profile.dart" % DART_RESERVED_DIR # Master
DART_ENVDIR           = "%s/env" % DART_RESERVED_DIR          # Master
DART_ENV              = "%s/env.dart" % DART_RESERVED_DIR     # Master/Slaves
DART_TEST_SCRIPTS_DIR = "%s/allscripts" % DART_RESERVED_DIR   # Master/Slaves
DART_TEST_RESULT      = "%s/testresult" % DART_RESERVED_DIR   # Master
DART_TEST_ERROR_MSG   = "%s/testerrmsg" % DART_RESERVED_DIR   # Master
