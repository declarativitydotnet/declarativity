// -*- c-basic-offset: 2; -*-
/*
 * @(#)$Id$
 *
 *  Utility functions to produce a DOT description of a router
 *  configuration.
 *
 *  Petros Maniatis.
 */

#ifndef __DOT_H__
#define __DOT_H__

#include <router.h>
#include <iostream>

void
toDot(std::ostream*, Router::ConfigurationPtr);

#endif /* __DOT_H__ */

