# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

bin_SCRIPTS = yappsrt.py

all: parsedesc.py

%.py: %.g yapps2.py yappsrt.py
	@PYTHON@ yapps2.py $<

CLEANFILES = *.pyc

EXTRA_DIST = parsedesc.g \
             yapps2.py yappsrt.py
