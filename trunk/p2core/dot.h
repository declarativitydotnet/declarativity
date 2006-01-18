// -*- c-basic-offset: 2; -*-
/*
 * @(#)$Id$
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

