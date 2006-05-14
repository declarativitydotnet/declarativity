/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION:
 */

#include "overlogCompiler.h"
#include <iostream>
#include "udp.h"
#include "ol_lexer.h"
#include "ol_context.h"
#include "plmb_confgen.h"
#include "val_str.h"

OverlogCompiler::OverlogCompiler(string n, PlumberPtr p, string id, string d) 
  : Element(n, 1, 1), _plumber(p), _id(id), _dataflow(d)
{
}

int OverlogCompiler::push(int port, TuplePtr tp, b_cbv cb)
{
  if (tp->size() > 2 && (*tp)[1]->typeCode() == Value::STR &&
      Val_Str::cast((*tp)[0]) == "overlog") {
    std::ostringstream script;
    ValuePtr dest    = (*tp)[1];
    ValuePtr source  = (*tp)[2];
    string   overlog = Val_Str::cast((*tp)[3]);

    compile(overlog, script);

    TuplePtr script_tuple = Tuple::mk();
    script_tuple->append(Val_Str::mk("script"));
    script_tuple->append(dest);
    script_tuple->append(source);
    script_tuple->append(Val_Str::mk(script.str())); 
    script_tuple->freeze();
    return output(0)->push(script_tuple, cb);
  }
  return output(0)->push(tp, cb);
}

void OverlogCompiler::compile(string overlog, std::ostringstream& script) {
  std::istringstream overlog_iss(overlog, std::istringstream::in);
  boost::shared_ptr< OL_Context > ctxt(new OL_Context());
  ctxt->parse_stream(&overlog_iss);

  Plumber::DataflowEditPtr conf = _plumber->new_dataflow_edit(_dataflow);
  Plmb_ConfGen *gen = new Plmb_ConfGen(ctxt.get(), conf, false, false, false, 
                                       string("overlogCompiler"), script, true);
  gen->createTables(_id);
  gen->configurePlumber(boost::shared_ptr<Udp>(), _id);
}

string OverlogCompiler::readScript( string fileName )
{
  string script = "";
  std::ifstream file;

  file.open( fileName.c_str() );

  if ( !file.is_open() )
  {
    std::cout << "Cannot open script file, \"" << fileName << "\"!" << std::endl;
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
