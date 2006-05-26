/**
 *
 * @(#)$Id$
 *
 * DESCRIPTION: Element which produces a trace tuple upon
 * seeing a particular tuple which is requested to be traced.
 *
 * Note that most of the functionality is similar to that
 * of the DuplicateConservative element.
 *
 * e.g. if trace(lookup),
 * then this element will look for "lookup" tuples and 
 * creates a tuple of form <intrace-lookup, <fields of lookup>, id>
 * id is the tuple id of "lookup" tuple
 */

#ifndef __TRACETUPLE_H__
#define __TRACETUPLE_H__

#include "element.h"


class TraceTuple : public Element {
 public:
  TraceTuple(string id, string tupleName);
  ~TraceTuple();

  /** Overridden to perform the tracing tuples */
  int push(int port, TuplePtr p, b_cbv cb);

  const char *class_name() const { return "TraceTuple";}
  const char *processing() const { return "a/a";}
  const char *flow_code()  const { return "-/-";}

 private:
  string _id;
  string _tupleName;

  /** The callback for my input **/
  b_cbv _push_cb;

  /** My block flags, one per output port **/
  std::vector<int> _block_flags;

  /** My block flag count **/
  unsigned int _block_flag_count;

  /** My block callback function for a given output **/
  void unblock(unsigned output);
};

#endif /** __TRACETUPLE_H__ **/
