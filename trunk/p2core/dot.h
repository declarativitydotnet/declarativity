// -*- c-basic-offset: 2; related-file-name: "dot.C" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 *  Utility functions to produce a DOT description of a plumber
 *  configuration.
 *
 *  Petros Maniatis.
 */

#ifndef __DOT_H__
#define __DOT_H__

#include <iostream>

void
toDot(std::ostream*, const std::set<ElementSpecPtr>&, const std::set<ElementSpec::HookupPtr>&);

#endif /* __DOT_H__ */

