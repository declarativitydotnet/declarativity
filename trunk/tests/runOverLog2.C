// -*- c-basic-offset: 2; related-file-name: "" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A chord dataflow.
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>
#include <sys/wait.h>

#include "tuple.h"
#include "p2.h"

P2::CallbackHandlePtr ping_handle;
P2 *p2;

string
readScript(string fileName,
           std::vector< std::string > definitions)
{
  string processed;
  if (fileName == "-") {
    processed = "stdout.processed";
  } else {
    processed = fileName + ".processed";
  }
  

  // Turn definitions vector into a cpp argument array.
  int defSize = definitions.size();
  char* args[defSize
             + 1                // for cpp
             + 2                // for flags -C and -P
             + 2                // for filenames
             + 1];              // for the null pointer at the end

  int count = 0;

  args[count++] = "cpp";
  args[count++] = "-P";
  args[count++] = "-C";

  for (std::vector< std::string>::iterator i =
         definitions.begin();
       i != definitions.end();
       i++) {
    args[count] = (char*) (*i).c_str();
    count++;
  }

  args[count++] = (char*) fileName.c_str();
  args[count++] = (char*) processed.c_str();
  args[count++] = NULL;


  // Invoke the preprocessor
  pid_t pid = fork();
  if (pid == -1) {
    TELL_ERROR << "Cannot fork a preprocessor\n";
    exit(1);
  } else if (pid == 0) {
    if (execvp("cpp", args) < 0) {
      TELL_ERROR << "CPP ERROR" << std::endl;
    }
    exit(1);
  } else {
    wait(NULL);
  }


  // Read processed script.
  std::ifstream file;
  file.open(processed.c_str());

  if (!file.is_open()) {
    TELL_ERROR << "Cannot open processed Overlog file \""
               << processed << "\"!\n";
    return std::string();
  } else {

    std::ostringstream scriptStream;
    std::string line;
    
    while(std::getline(file, line)) {
      scriptStream << line << "\n";
    }

    file.close();
    std::string script = scriptStream.str();


    return script;
  }
}

static char* USAGE = "Usage:\n\t runOverLog2\n"
                     "\t\t[-o <overLogFile> (default: standard input)]\n"
                     "\t\t[-r <loggingLevel> (default: ERROR)]\n"
                     "\t\t[-n <myipaddr> (default: localhost)]\n"
                     "\t\t[-p <port> (default: 10000)]\n"
                     "\t\t[-D<key>=<value>]*\n"
                     "\t\t[-h (gets usage help)]\n";

int main(int argc, char **argv)
{
  string overLogFile("-");
  string myHostname = "localhost";
  int port = 10000;
  std::string portString("10000");
  std::vector< std::string > definitions;

  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, "o:r:n:p:D:h")) != -1) {
    switch (c) {
    case 'o':
      overLogFile = optarg;
      break;

    case 'r':
      {
        // My minimum reporting level is optarg
        std::string levelName(optarg);
        Reporting::Level level =
          Reporting::levelFromName[levelName];
        Reporting::setLevel(level);
      }
      break;

    case 'n':
      myHostname = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      portString = string(optarg);
      break;

    case 'D':
      definitions.push_back(std::string("-D") + optarg);
      break;

    case 'h':
    default:
      TELL_ERROR << USAGE;
      exit(-1);
    }
  }
  
  TELL_INFO << "Running from translated file \"" << overLogFile << "\"\n";

  std::ostringstream myAddressBuf;
  myAddressBuf <<  myHostname << ":" << port;
  std::string myAddress = myAddressBuf.str();
  TELL_INFO << "My address is \"" << myAddress << "\"\n";
  
  TELL_INFO << "My environment is ";
  for (std::vector< std::string>::iterator i =
         definitions.begin();
       i != definitions.end();
       i++) {
    TELL_INFO << (*i) << " ";
  }
  TELL_INFO << "\n";

  string program(readScript(overLogFile,
                            definitions));

  p2 = new P2(myHostname, portString,
              P2::NONE);

  TELL_INFO << "INSTALLING PROGRAM" << std::endl;
  p2->install("overlog", program);
  
  p2->run();

  return 0;
}
  

/*
 * End of file 
 */
