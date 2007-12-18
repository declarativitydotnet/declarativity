/**
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776,
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
*/


#include "execRecord.h"
#include "val_time.h"

ExecRecord::ExecRecord(string rule)
{
  initializePreds();
  ruleId = rule;
  range_start = 0;
  range_end = 0;
  remoteNode = "-";
  numEntries = 0;
  tupleIn = 0;
  tupleOut = 0;
  finished = false;
  resetTimer(&timeIn);
  resetTimer(&timeOut);
}

void ExecRecord::initializePreds()
{
  for(int i = 0; i < MAX_STAGES; i++)
    precond[i] = 0;
  return;

}

string ExecRecord::toString()
{

  ostringstream os;
  os << "LOG-ENTRY: Start\n";
  os << "Rule " << ruleId << "\n";
  os << "Tin " << tupleIn << "\n";
  os << "Tout " << tupleOut << "\n";
  if(numEntries > MAX_STAGES){
    TELL_ERROR << "NUmber of preconditions is " << numEntries << ", " << os.str() << "\n";
    std::exit(-1);
  }
  for(int i = 0; i < numEntries; i++){
    os << "Precondition " << i << " " << precond[i] << "\n";
  }
  os << "LOG-ENTRY: End\n";
  return os.str();
}

void ExecRecord::flushTillIndex(int index)
{
   for(int i = index; i < MAX_STAGES; i++){
    if(precond[index] != 0){
      precond[i] = 0;
      numEntries--;
    }
  }
  tupleOut = 0;
  resetTimer(&timeOut);
  return;

}

void ExecRecord::flush()
{
  tupleIn = tupleOut = 0;
  finished = false;
  resetTimer(&timeIn);
  resetTimer(&timeOut);
  numEntries = 0;
  range_start = range_end = 0;
  remoteNode = "-";
  initializePreds();
  return;

}



