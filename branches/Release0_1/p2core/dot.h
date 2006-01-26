// -*- c-basic-offset: 2; -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 *
 *  Utility functions to produce a DOT description of a plumber
 *  configuration.
 *
 *  Petros Maniatis.
 */

#ifndef __DOT_H__
#define __DOT_H__

#include <plumber.h>
#include <iostream>

void
toDot(std::ostream*, Plumber::ConfigurationPtr);

#endif /* __DOT_H__ */

