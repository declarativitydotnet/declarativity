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
 *
 */

#include<iostream>
#include "compoundContext.h"
#include "plumber.h"
#include "systemTable.h"
#include "value.h"
#include "val_str.h"
#include "val_null.h"
#include "val_uint32.h"
#include "val_int32.h"
#include "val_list.h"
#include "val_tuple.h"
namespace compile {
  namespace compound{
    using namespace opr;

    DEFINE_ELEMENT_INITS_NS(Context, "CompoundContext", compile::compound)

    Context::Context(string name)
    : compile::Context(name) { }

    Context::Context(TuplePtr args)
    : compile::Context((*args)[2]->toString()) { }

    void
    Context::rule(CommonTable::ManagerPtr catalog, TuplePtr rule)
    {
      static int counter = 0;
      std::cout<<"compound stage rule counter"<<counter++<<std::endl;
      //do nothing
    } 
  
    TuplePtr 
    Context::program(CommonTable::ManagerPtr catalog, TuplePtr program) 
    {
      std::cout<<"Reached compound stage"<<std::endl;
      CommonTable::Key indexKey;
      CommonTable::Iterator iter;

      indexKey.push_back(catalog->attribute(REF, "PID"));
      iter =
        catalog->table(REF)->lookup(CommonTable::theKey(CommonTable::KEY2), indexKey, program); 
      while (!iter->done()) {
        TuplePtr ref = iter->next();                                        // The row in the fact table
	std::cout<<"Ref:"<<ref->toString()<<std::endl;
      }

      std::cout<<"exitting compound stage"<<std::endl;
      return this->compile::Context::program(catalog, program);
    }
 
  }
}
