// -*- c-basic-offset: 2; related-file-name: "netLoader.h" -*-
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
 */

#include "langLoader.h"


#include "ecaContext.h"
#include "p2dlContext.h"
#include "debugContext.h"
#include "localContext.h"
#include "parseContext.h"
#include "plannerContext.h"
#include "rewriteContext.h"


void
LangLoader::loadElements()
{
  compile::eca::Context::ensureInit();
  compile::p2dl::Context::ensureInit();
  compile::debug::Context::ensureInit();
  compile::local::Context::ensureInit();
  compile::parse::Context::ensureInit();
  compile::planner::Context::ensureInit();
  compile::rewrite::Context::ensureInit();
}

