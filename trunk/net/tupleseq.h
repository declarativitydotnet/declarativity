// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __TupleSeq_H__
#define __TupleSeq_H__

#include "element.h"
#include "val_uint64.h"

class TupleSeq {
public:

  class Package : public Element {
  public:
    Package(str name, u_int64_t start_seq);
    const char *class_name() const	{ return "TupleSeq::Package";};
    const char *processing() const	{ return "a/a"; };
    const char *flow_code() const	{ return "x/x"; };

    TuplePtr simple_action(TupleRef p);	// Adds the next sequence num to tuple stream. 

  private:
    u_int64_t seq_;
  };

  class Unpackage : public Element {
  public:
    Unpackage(str name);
    const char *class_name() const	{ return "TupleSeq::Unpackage";};
    const char *processing() const	{ return "a/a"; };
    const char *flow_code() const	{ return "x/x"; };

    TuplePtr simple_action(TupleRef p);	// Removes sequence num from tuple stream. 
  };

  //
  // Now the TupleSeq object itself.
  //
  TupleSeq(str name="TupleSeq", u_int64_t start_seq=0);

  // Accessing the individual elements
  ref< TupleSeq::Package >   get_package()   { return pack_; };
  ref< TupleSeq::Unpackage > get_unpackage() { return unpack_; };

private:
  // Elements 
  ref< Package >   pack_;
  ref< Unpackage > unpack_;
};

#endif /* __TupleSeq_H_ */
