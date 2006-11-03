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

string readScript( string fileName, char* args[] )
{
  string script = "";
  std::ifstream file;

  string processed(fileName+".processed");
  int i = 0;
  for (i = 0; args[i] != (char*)NULL; i++)
    ;
  args[i++] = (char*) fileName.c_str();
  args[i]   = (char*) processed.c_str();

  pid_t pid = fork();
  if (pid == -1) {
    TELL_ERROR << "Cannot fork a preprocessor\n";
    exit(1);
  } 
  else if (pid == 0) {
    if (execvp("cpp", args) < 0) {
      TELL_ERROR << "CPP ERROR" << std::endl;
    }
    exit(1);
  }
  else {
    wait(NULL);
  }
  file.open( processed.c_str() );

  if ( !file.is_open() )
  {
    TELL_ERROR << "Cannot open Overlog file, \"" << processed << "\"!" << std::endl;
    return script;
  }
  else
  {
    // Get the length of the file
    file.seekg( 0, std::ios::end );
    int nLength = file.tellg();
    file.seekg( 0, std::ios::beg );

    // Allocate  a char buffer for the read.
    char *buffer = new char[nLength];
    memset( buffer, 0, nLength );

    // read data as a block:
    file.read( buffer, nLength );

    script.assign( buffer );

    delete [] buffer;
    file.close();
    return script;
  }

  pid = fork();
  if (pid == -1) {
    TELL_ERROR << "Cannot fork a preprocessor\n";
    exit(1);
  } 
  else if (pid == 0) {
    if (execlp("rm", "rm", "-f", processed.c_str(), (char*) NULL) < 0)
      TELL_ERROR << "CPP ERROR" << std::endl;
    exit(1);
  }
  else {
    wait(NULL);
  }
  return script;
}

void watch(TuplePtr tp)
{
  TELL_INFO << tp->toString() << std::endl;
}

void print_usage()
{
  TELL_ERROR << "Usage: runOverlog [-Dvariable=value [-Dvariable=value [...]]]\n " 
            << "                 [-w tupleName [-w tupleName [...]]]\n "
            << "                 -r <reporting level>\n"
            << "                 <overlogFile> <hostname> <port>" << std::endl;
}

int main(int argc, char **argv)
{
  if (argc < 4) {
    print_usage();
    exit(-1);
  }
  // Skip the program name
  argc--;
  argv++;

  char* args[argc+4];
  for (int i = 0; i < argc+4; i++)
    args[i] = (char*)NULL;
  args[0] = "cpp";
  args[1] = "-P";

  std::vector<string> watchTuples;
  for (int a = 2; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
    if (argv[0][1] == 'D') {
      args[a++] = argv[0];  
    } else if (argv[0][1] == 'w') {
      if (argv[0][2] == '\0') {
        argc--; argv++;
        watchTuples.push_back(string(&argv[0][0]));      
      }
      else {
        watchTuples.push_back(string(&argv[0][2]));      
      }
    } else if (argv[0][1] == 'r') {
      string levelName(argv[1]);
      Reporting::Level level =
        Reporting::levelFromName[levelName];
      Reporting::setLevel(level);
      argc--;
      argv++;
    } else {
      print_usage();
      exit(-1);
    }
  }

  string program(readScript(argv[0], &args[0]));
  string hostname(argv[1]);
  string port(argv[2]);
  p2 = new P2(hostname, port,
              P2::NONE);

  TELL_INFO << "INSTALL PROGRAM" << std::endl;
  p2->install("overlog", program);
  
  for (std::vector<string>::iterator iter = watchTuples.begin(); 
       iter != watchTuples.end(); iter++) {
    p2->subscribe(*iter, boost::bind(&watch, _1));
  }
  p2->run();

  return 0;
}
  

/*
 * End of file 
 */
