## This file is distributed under the terms in the attached LICENSE file.
## If you do not find this file, copies can be found by writing to:
## Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
## Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
## Or
## UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
## Berkeley, CA,  94707. Attention: P2 Group.
## 
## DESCRIPTION: Design of the P2 service interface, the external view of
## P2 to its users.
## 
## $Id$
## 

OVERVIEW
--------


The purpose of this document is to specify the external service
interface of P2 to its users. This interface is to encompass all the
(known) methods of interaction with P2, programmatic or command-line,
that P2 developers intend to maintain currently.

The latest system contains the following modes of user interaction:

1. Installation and execution of a single, stand-alone OverLog program.
   This is currently performed via the tests/runOverLog2 utility.

2. Installation and execution of an updatable OverLog program. Also
   currently performed via the tests/runOverLog2 utility.

3. Incremental upload of an OverLog program to a running system. The
   upload is performed via the python/scripts/p2terminal.py script,
   which interfaces with a system started via tests/runOverLog2.

4. Programmatic installation of event handlers, written in C++, or
   Python, connected to a single, updatable OverLog program. This is
   done by writing a program in C++ or Python that creates a P2 object
   and registers event handlers with that P2 instance.

5. Dissemination of an updatable OverLog program to a PlanetLab
   slice. This is currently done via the python/scripts/psetup.py
   script, which uses python/scripts/planetlab.py as the driver program
   (as per bullet 4 above).

6. Dissemination of an updatable OverLog program to a number of
   processes running on a local machine.  This is currently done via
   tests/runManyOverLogs.  Updates to the process cluster are uploaded,
   as above, using python/scripts/p2terminal.py.

7. Unit tests of the existing, tested components. This is done via the
   unitTest/p2Test program.

8. Installation of a dataflow graph specified in P2DL. This is currently
   done via the tests/runOverLog2

Details on each of these modes of interaction with P2 follow below,
along with simple "how-to" examples for each.






IMPORTANT NOTICE
----------------

Please note that most python scripts under python/scripts are generated
by the make utility.  If you wish to alter any of these scripts, please
change the file ending in ".in" as opposed to the file ending in ".py".





