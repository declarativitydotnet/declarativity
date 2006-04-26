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

using namespace boost::python;
using namespace boost::python::api;

DataflowInstaller::DataflowInstaller(string n, PlumberPtr p, object o) 
  : Element(n, 1,0), plumber_(p), parser_(o)
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

int DataflowInstaller::push(int port, TuplePtr, b_cbv cb)
{
  // Send as many more tuples as you want
  return 1;
}

void DataflowInstaller::install(Plumber::DataflowPtr dataflow) {

}

void DataflowInstaller::install(string fileName) {
  std::cerr << "INSTALLING: " << fileName << std::endl;
        
  string script = readScript(fileName);
  tuple result   = extract<tuple>(parser_.attr("compile")(plumber_, script));
  dict dataflows = extract<dict>(result[0]);
  list edits     = extract<list>(result[1]);

  int ndataflows = extract<int>(dataflows.attr("__len__")());
  int nedits = extract<int>(edits.attr("__len__")());

  std::cerr << "PARSED " << ndataflows << " DATAFLOWS\n";
  std::cerr << "PARSED " << nedits << " EDITS\n";

  for (int i = 0; i < ndataflows; i++) {
    tuple t = dataflows.popitem();
    char* name = extract<char*>(t[0]);
    std::cerr << "DATAFLOW " << i << ": " << name << std::endl;
    t[1].attr("eval_dataflow")();
    Plumber::DataflowPtr d = extract<Plumber::DataflowPtr>(t[1].attr("conf"));
    if (plumber_->install(d) < 0) {
      std::cerr << "DATAFLOW INSTALLATION FAILURE FOR " << name << std::endl;
      exit(0);
    }
  }
  for (int i = 0; i < nedits; i++) {
    Plumber::DataflowEditPtr e = extract<Plumber::DataflowEditPtr>(edits[i]);
    if (plumber_->install(e) < 0) {
      std::cerr << "EDIT INSTALLATION FAILURE FOR " << e->name() << std::endl;
      exit(0);
    }
  }
  plumber_->toDot("dataflowInstaller.dot");
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
