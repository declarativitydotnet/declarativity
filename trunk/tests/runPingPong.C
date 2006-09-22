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

#include "tuple.h"
#include "val_str.h"
#include "p2.h"

P2::CallbackHandlePtr ping_handle;
P2 *p2;

string readScript( string fileName )
{
  string script = "";
  std::ifstream file;

  file.open( fileName.c_str() );

  if ( !file.is_open() )
  {
    std::cout << "Cannot open Ping Overlog file, \"" << fileName << "\"!" << std::endl;
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
}

TuplePtr tup()
{
  TuplePtr tp = Tuple::mk();
  tp->append(Val_Str::mk("test"));
  tp->append(Val_Str::mk("THIS IS A TEST"));
  tp->freeze();
  return tp;
}

void ping_cb(TuplePtr tp)
{
  std::cerr << "RECEIVED PING TUPLE: " << tp->toString() << std::endl;
  // std::cerr << "CANCELING CALLBACK: " << std::endl;
  // p2->unsubscribe(ping_handle); 
  // p2->tuple(tup());
}

void test_cb(TuplePtr tp)
{
  std::cerr << "RECEIVED TEST TUPLE: " << tp->toString() << std::endl;
  ping_handle = p2->subscribe("ping", boost::bind(&ping_cb, _1));
}

int main(int argc, char **argv)
{
  if (argc < 4) {
    std::cerr << "Usage:\n\t runPingPong <pingPongDatalogFile> <hostname> <port>" << std::endl;
    exit(-1);
  }
  
  string ping(readScript(argv[1]));
  string hostname(argv[2]);
  string port(argv[3]);
  p2 = new P2(hostname, port,
              P2::ORDERED | P2::RCC | P2::RELIABLE,
              LoggerI::NONE);

  p2->install("overlog", ping);
  std::cerr << "INSTALLED OVERLOG" << std::endl;
  ping_handle = p2->subscribe("ping", boost::bind(&ping_cb, _1));
  p2->subscribe("test", boost::bind(&test_cb, _1));
  p2->run();

  return 0;
}
  

/*
 * End of file 
 */
