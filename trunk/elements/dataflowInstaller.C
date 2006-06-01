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
#include "val_tuple.h"
#include "val_uint32.h"
#include "val_int32.h"
#include "ID.h"
#include "val_id.h"

#define HACK

using namespace boost::python;
using namespace boost::python::api;

DataflowInstaller::DataflowInstaller(string n, PlumberPtr p, 
                                     object o, string local, string landmark) 
  : Element(n, 1, 1), plumber_(p), parser_(o), 
    localAddress_(local), landmarkAddress_(landmark)
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
      Val_Str::cast((*tp)[0]) == "script") {
    ostringstream mesg;
    ValuePtr my_addr    = (*tp)[1];
    ValuePtr other_addr = (*tp)[2];
    string   script = Val_Str::cast((*tp)[3]);
    int      result = install(script, mesg);

    std::cerr << mesg.str() << std::endl;

    TuplePtr packaged = Tuple::mk();
    TuplePtr status   = Tuple::mk();
    packaged->append(other_addr);
    status->append(Val_Str::mk("status"));
    status->append(my_addr);
    status->append(Val_Int32::mk(result));
    status->append(Val_Str::mk(mesg.str())); 
    status->freeze();
    packaged->append(Val_Tuple::mk(status));
    packaged->freeze();
    return output(0)->push(packaged, cb);
  }
  return output(0)->push(tp, cb);
}

int DataflowInstaller::install(string script, ostringstream& status) {
  parser_.attr("clear")();
  tuple result   = extract<tuple>(parser_.attr("compile")(plumber_, script));
  dict dataflows = extract<dict>(result[0]);
  list edits     = extract<list>(result[1]);

  int ndataflows = extract<int>(dataflows.attr("__len__")());
  int nedits = extract<int>(edits.attr("__len__")());

  status << "Dataflow Installer Status" << std::endl;
  if (ndataflows) status << "SUCCESSFULLY PARSED " << ndataflows << " DATAFLOWS\n";
  if (nedits) status << "SUCCESSFULLY PARSED " << nedits << " EDITS\n";

  for (int i = 0; i < ndataflows; i++) {
    tuple t = dataflows.popitem();
    char* name = extract<char*>(t[0]);
    t[1].attr("eval_dataflow")();
    Plumber::DataflowPtr d = extract<Plumber::DataflowPtr>(t[1].attr("conf"));
    if (plumber_->install(d) < 0) {
      status << "DATAFLOW INSTALLATION FAILURE FOR " << name << std::endl;
      return -1;
    }
    else {
      status << "SUCCESSFUL INSTALLATION FOR DATAFLOW: " << name << std::endl;
    }
  }
  for (int i = 0; i < nedits; i++) {
    edits[i].attr("eval_dataflow")();
    Plumber::DataflowEditPtr e = extract<Plumber::DataflowEditPtr>(edits[i].attr("conf"));
#ifdef HACK
    initializeChordBaseTables(e);
#endif
 
    if (plumber_->install(e) < 0) {
      status << "EDIT INSTALLATION FAILURE FOR " << e->name() << std::endl;
      return -1;
    }
    else {
      status << "SUCCESSFUL INSTALLATION FOR DATAFLOW EDIT: " << e->name() << std::endl;
    }
  }
  plumber_->toDot("dataflowInstaller.dot");
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

void
DataflowInstaller::initializeChordBaseTables(Plumber::DataflowPtr d) {
  // create information on the node itself  
  if (d->getTable("node") != 0 && d->getTable("node")->size() == 0) {
    uint32_t random[ID::WORDS];
    for (uint32_t i = 0; i < ID::WORDS; i++) {
      random[i] = rand();
    }
  
    IDPtr myKey = ID::mk(random);
    Table2Ptr nodeTable = d->table("node", Table2::KEY1);
    TuplePtr tuple = Tuple::mk();
    tuple->append(Val_Str::mk("node"));
    tuple->append(Val_Str::mk(localAddress_));
    tuple->append(Val_ID::mk(myKey));
    tuple->freeze();
    nodeTable->insert(tuple);
    warn << "Node: " << tuple->toString() << "\n";
  }

  if (d->getTable("pred") != 0 && d->getTable("pred")->size() == 0) {
    Table2Ptr predecessorTable = d->table("pred", Table2::KEY1);
    TuplePtr predecessorTuple = Tuple::mk();
    predecessorTuple->append(Val_Str::mk("pred"));
    predecessorTuple->append(Val_Str::mk(localAddress_));
    predecessorTuple->append(Val_ID::mk(ID::mk()));
    predecessorTuple->append(Val_Str::mk(string("-"))); 
    predecessorTuple->freeze();
    predecessorTable->insert(predecessorTuple);
    warn << "Initial predecessor " << predecessorTuple->toString() << "\n";
  }

  if (d->getTable("nextFingerFix") != 0 && d->getTable("nextFingerFix")->size() == 0) {
    Table2Ptr nextFingerFixTable = d->table("nextFingerFix", Table2::KEY1);
    TuplePtr nextFingerFixTuple = Tuple::mk();
    nextFingerFixTuple->append(Val_Str::mk("nextFingerFix"));
    nextFingerFixTuple->append(Val_Str::mk(localAddress_));
    nextFingerFixTuple->append(Val_UInt32::mk(0));
    nextFingerFixTuple->freeze();
    nextFingerFixTable->insert(nextFingerFixTuple);
    warn << "Next finger fix: " << nextFingerFixTuple->toString() << "\n";
  }

  if (d->getTable("landmark") != 0 && d->getTable("landmark")->size() == 0) {
    Table2Ptr landmarkNodeTable = d->table("landmark", Table2::KEY1);  
    TuplePtr landmark = Tuple::mk();
    landmark->append(Val_Str::mk("landmark"));
    landmark->append(Val_Str::mk(localAddress_));
    landmark->append(Val_Str::mk(landmarkAddress_));
    landmark->freeze();
    warn << "Insert landmark node " << landmark->toString() << "\n";
    landmarkNodeTable->insert(landmark);
  }
}
