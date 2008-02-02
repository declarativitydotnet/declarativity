// -*- c-basic-offset: 2; related-file-name: "elementLoader.h" -*-
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

#include "elementLoader.h"

#include "aggregate.h"
#include "aggwrap2.h"
#include "aggwrap2Callback.h"
#include "commitBuf.h"
#include "compileStage.h"
#include "stage.h"
#include "programLoader.h"
#include "ddemux.h"
#include "dDuplicateConservative.h"
#include "delete2.h"
#include "delete.h"
#include "demuxConservative.h"
#include "demux.h"
#include "discard.h"
#include "dRoundRobin.h"
#include "dupElim.h"
#include "duplicateConservative.h"
#include "duplicate.h"
#include "elementRegistry.h"
#include "staticTupleSource.h"
#include "hexdump.h"
#include "identity.h"
#include "insert2.h"
#include "insert.h"
#include "logger.h"
#include "lookup2.h"
#include "marshalField.h"
#include "marshal.h"
#include "mux.h"
#include "noNullField.h"
#include "noNull.h"
#include "onlyNullField.h"
#include "noNullSignal.h"
#include "pelTransform.h"
#include "print.h"
#include "tupleCounter.h"
#include "printTime.h"
#include "printWatch.h"
#include "pullPush.h"
#include "queue.h"
#include "rangeLookup.h"
#include "refresh.h"
#include "removed.h"
#include "roundRobin.h"
#include "slot.h"
#include "strToSockaddr.h"
#include "switch.h"
#include "timedPullPush.h"
#include "tupleInjector.h"
#include "unboxField.h"
#include "unmarshalField.h"
#include "unmarshal.h"
#include "update.h"


void
ElementLoader::loadElements()
{
  Aggregate::ensureInit();
  Aggwrap2::ensureInit();
  Aggwrap2Callback::ensureInit();
  CommitBuf::ensureInit();
  CompileStage::ensureInit();
  Stage::ensureInit();
  ProgramLoader::ensureInit();
  DDemux::ensureInit();
  DDuplicateConservative::ensureInit();
  Delete2::ensureInit();
  Delete::ensureInit();
  Demux::ensureInit();
  DemuxConservative::ensureInit();
  Discard::ensureInit();
  DRoundRobin::ensureInit();
  DupElim::ensureInit();
  Duplicate::ensureInit();
  DuplicateConservative::ensureInit();
  Hexdump::ensureInit();
  Identity::ensureInit();
  Insert2::ensureInit();
  Insert::ensureInit();
  Logger::ensureInit();
  Lookup2::ensureInit();
  Marshal::ensureInit();
  MarshalField::ensureInit();
  Mux::ensureInit();
  NoNull::ensureInit();
  NoNullField::ensureInit();
  OnlyNullField::ensureInit();
  NoNullSignal::ensureInit();
  PelTransform::ensureInit();
  Print::ensureInit();
  TupleCounter::ensureInit();
  PrintTime::ensureInit();
  PrintWatch::ensureInit();
  PullPush::ensureInit();
  Queue::ensureInit();
  RangeLookup::ensureInit();
  Refresh::ensureInit();
  Removed::ensureInit();
  RoundRobin::ensureInit();
  Slot::ensureInit();
  StrToSockaddr::ensureInit();
  Switch::ensureInit();
  TimedPullPush::ensureInit();
  StaticTupleSource::ensureInit();
  TupleInjector::ensureInit();
  UnboxField::ensureInit();
  Unmarshal::ensureInit();
  UnmarshalField::ensureInit();
  Update::ensureInit();
}

