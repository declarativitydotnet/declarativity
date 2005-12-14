/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 *
 */

#ifndef __AGG_H__
#define __AGG_H__

#include "element.h"
#include <queue>

class Agg : public Element { 
public:

  Agg(str name, std::vector<unsigned int> groupByFields, 
      int aggField, unsigned uniqueField, str aggStr);

  ~Agg();
  
  int push(int port, TupleRef p, b_cbv cb);
  TuplePtr pull(int port, b_cbv cb);  

  const char *class_name() const		{ return "Agg";}
  const char *processing() const		{ return "h/l"; }
  const char *flow_code() const			{ return "x/x"; }

private:
  str getGroupByFields(TupleRef p);
  bool checkBestTuple(TupleRef p);
  void updateBestTuple(TupleRef p);

  b_cbv _pullCB, _pushCB;
  std::map<str, TupleRef> _buffer; // best values to send 
  std::map<str, TupleRef> _bestSoFar; // best tuple so far
  std::vector<unsigned int> _groupByFields;
  std::map<str, TupleRef> _allValues;
  int _aggField;
  unsigned _uniqueField;
  str _aggStr;
};


#endif 
