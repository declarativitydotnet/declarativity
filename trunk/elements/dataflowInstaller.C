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

#include "dataflowInstaller.h"
#include <iostream>
#include <fstream>
#include "val_str.h"
#include "val_int32.h"

using namespace boost::python;
using namespace boost::python::api;

DataflowInstaller::DataflowInstaller(string n, PlumberPtr p, object o) 
  : Element(n, 1, 1), plumber_(p), parser_(o)
{
  // Start python and instantiate dfparser object I'm not given one
  if (o.ptr() == object().ptr()) {
    std::cerr << "NO PARSER PROVIDED: MAKE ONE" << std::endl;
    Py_Initialize();
    object parser(handle<> (borrowed(PyImport_ImportModule("dfparser"))));
    parser_ = parser;
  }
  if (parser_.ptr() == object().ptr())
    std::cerr << "UNABLE TO IMPORT DFPARSER" << std::endl;
}

int DataflowInstaller::push(int port, TuplePtr tp, b_cbv cb)
{
  if (tp->size() > 2 && (*tp)[1]->typeCode() == Value::STR &&
      Val_Str::cast((*tp)[1]) == "overlog") {
    ostringstream mesg;
    ValuePtr source = (*tp)[2];
    string   script = Val_Str::cast((*tp)[2]);
    int      result = install(script, mesg);

    TuplePtr status = Tuple::mk();
    status->append(source);
    status->append(Val_Int32::mk(result));
    status->append(Val_Str::mk(mesg.str())); 
    status->freeze();
    return output(0)->push(status, cb);
  }
  return output(0)->push(tp, cb);
}

int DataflowInstaller::install(string script, ostringstream& status) {
  tuple result   = extract<tuple>(parser_.attr("compile")(plumber_, script));
  dict dataflows = extract<dict>(result[0]);
  list edits     = extract<list>(result[1]);

  int ndataflows = extract<int>(dataflows.attr("__len__")());
  int nedits = extract<int>(edits.attr("__len__")());

  status << "PARSED " << ndataflows << " DATAFLOWS\n";
  status << "PARSED " << nedits << " EDITS\n";

  for (int i = 0; i < ndataflows; i++) {
    tuple t = dataflows.popitem();
    char* name = extract<char*>(t[0]);
    std::cerr << "DATAFLOW " << i << ": " << name << std::endl;
    t[1].attr("eval_dataflow")();
    Plumber::DataflowPtr d = extract<Plumber::DataflowPtr>(t[1].attr("conf"));
    if (plumber_->install(d) < 0) {
      status << "DATAFLOW INSTALLATION FAILURE FOR " << name << std::endl;
      return -1;
    }
  }
  for (int i = 0; i < nedits; i++) {
    Plumber::DataflowEditPtr e = extract<Plumber::DataflowEditPtr>(edits[i]);
    if (plumber_->install(e) < 0) {
      status << "EDIT INSTALLATION FAILURE FOR " << e->name() << std::endl;
      return -1;
    }
  }
  // plumber_->toDot("dataflowInstaller.dot");
  return 0;
}

string DataflowInstaller::readScript( string fileName )
{
  string script = "";
  std::ifstream pythonFile;

  pythonFile.open( fileName.c_str() );

  if ( !pythonFile.is_open() )
  {
    std::cout << "Cannot open Python script file, \"" << fileName << "\"!" << std::endl;
    return script;
  }
  else
  {
    // Get the length of the file
    pythonFile.seekg( 0, std::ios::end );
    int nLength = pythonFile.tellg();
    pythonFile.seekg( 0, std::ios::beg );

    // Allocate  a char buffer for the read.
    char *buffer = new char[nLength];
    memset( buffer, 0, nLength );

    // read data as a block:
    pythonFile.read( buffer, nLength );

    script.assign( buffer );

    delete [] buffer;
    pythonFile.close();

    return script;
  }
}
