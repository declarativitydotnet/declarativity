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

#include "netLoader.h"


#include "basicAck.h"
#include "bw.h"
#include "cct.h"
#include "cumulativeAck.h"
#include "defrag.h"
#include "dupRemove.h"
#include "frag.h"
#include "odelivery.h"
#include "rccr.h"
#include "rcct.h"
#include "rdelivery.h"
#include "roundTripTimer.h"
#include "tupleseq.h"
#include "udp2.h"
#include "udp.h"


void
NetLoader::loadElements()
{
  BasicAck::ensureInit();
  Bandwidth::ensureInit();
  CCT::ensureInit();
  CumulativeAck::ensureInit();
  Defrag::ensureInit();
  DupRemove::ensureInit();
  Frag::ensureInit();
  ODelivery::ensureInit();
  RateCCR::ensureInit();
  RateCCT::ensureInit();
  RDelivery::ensureInit();
  RoundTripTimer::ensureInit();
  Sequence::ensureInit();
  Udp::ensureInit();
  Udp2::ensureInit();
}

