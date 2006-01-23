// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 *
 */

#ifndef __GROUPBY_H__
#define __GROUPBY_H__

#include "element.h"
#include <map>
#include <vector>

/** Element that accepts tuples as push inputs, groups them by their group by fields, and periodically 
 generates all the group by fields and their aggregate values to be pushed upstream.

TO fix:
- For now, all incoming tuples are stored in an in memory table indexed by the grouping fields. 
- Tuples are not time-based for now. 
- Support min/max only
 */
class GroupBy : public Element { 
public:
  static const int MIN_AGG;
  static const int MAX_AGG;
  static const int AVG_AGG;

  GroupBy(string name, string newTableName, std::vector<int> primaryFields, std::vector<int> groupByFields, 
	  std::vector<int> aggFields, std::vector<int> aggTypes, double seconds, bool aggregateSelections);

  ~GroupBy();

  virtual int initialize();
  
  int push(int port, TuplePtr p, b_cbv cb);

  const char *class_name() const		{ return "GroupBy";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const			{ return "x/x"; }

private:
  string getFieldStr(std::vector<int> fields, TuplePtr p);
  void recomputeAllAggs();
  void dumpTuples(string str);
  void dumpAggs(string str);

  typedef std::multimap<string, TuplePtr> TupleMultiMap;
  typedef std::map<string, TuplePtr> TupleMap;

  // tuples, indexed by groupByField
  TupleMultiMap _tuples; 

  // agg values, indexed by groupByFields
  TupleMap _aggValues, _bestTuples;

  // the field no of the primary fields for tuple
  std::vector<int> _primaryFields; 

  // the field no of the group by fields
  std::vector<int> _groupByFields;

  // the fields to aggregate on
  std::vector<int> _aggFields;

  // what types of aggregates
  std::vector<int> _aggTypes;

  TupleMap _lastSentTuples; 

  // name of new agg tuples
  string _newTableName;

  /** The interval in seconds */
  double _seconds;

  /** My current iterators */
  TupleMultiMap::iterator _multiIterator;
  TupleMap::iterator _iterator;

  /** My wakeup callback */
  b_cbv _wakeupCB;

  /** Callback to my runTimer() */
  b_cbv _runTimerCB;

  /** My time callback ID. */
  timeCBHandle * _timeCallback;

  /** My wakeup method */
  void wakeup();
  void runTimer();

  bool _aggregateSelections;

  int _numAdded;
};


#endif /* __GROUPBY_H_ */
