#
#  This file is distributed under the terms in the attached LICENSE file.
#  If you do not find this file, copies can be found by writing to:
#  Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
#  Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
#
#!/usr/bin/env python

import os.path
import sys
import string
import tempfile

separator = "OUTPUT:"
test_dir = os.path.join(os.path.dirname(sys.argv[0]), 'ol_parser_tests')
overlog_binary = os.path.join(os.path.dirname(sys.argv[0]), 'overlog')
for t in [t[:-4] for t in os.listdir(test_dir) if t [-4:] == '.tst']:
  print "Running test '%s'..." % t
  # Strip off the trailing '\n'...
  lines = [l[:-1] for l in file(os.path.join(test_dir,t)+".tst").readlines()]
  sep = lines.index(separator)
  input_stuff = string.join(lines[:sep],'\n')+'\n'
  in_file, in_filename = tempfile.mkstemp()
  print in_file
  os.fdopen(in_file).write(input_stuff)
  out_file, out_filename = tempfile.mkstemp()
  err_file, err_filename = tempfile.mkstemp()
  out_file.close()
  in_file.close()
  err_file.close()
  os.system('%s -c %s 2>%s >%s' % ( overlog_binary,
                                    in_filename,
                                    err_filename,
                                    out_filename ))

  expect_stuff = string.join(lines[sep+1:],'\n')
  error_stuff = file(err_filename).read()
  output_stuff = file(out_filename).read()
  os.unlink(in_filename)
  os.unlink(out_filename)
  os.unlink(err_filename)
  if error_stuff.strip() != expect_stuff.strip():
    print "FAIL: ", t
    print "INPUT:", input_stuff
    print "OUTPUT:", output_stuff
    print "EXPECT:", expect_stuff
    print "ERROR:", error_stuff
    
  
  
  
  
