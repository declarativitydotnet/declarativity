// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __Bandwidth_H__
#define __Bandwidth_H__

#include <sys/time.h>
#include "element.h"

class Bandwidth : public Element {
public:

    Bandwidth(string name="Bandwidth");
    const char *class_name() const	{ return "Bandwidth";};
    const char *processing() const	{ return "a/a"; };
    const char *flow_code() const	{ return "x/x"; };

    TuplePtr simple_action(TuplePtr p);	// Adds the next sequence num to tuple stream. 

    operator double() { return bw_; };

private:
    REMOVABLE_INLINE time_t now_s() const;

    time_t prev_t_;
    unsigned int bytes_;
    double bw_;
};

#endif /* __Bandwidth_H_ */
