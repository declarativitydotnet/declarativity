
/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Network planner code
 *
 */

#include "netPlanner.h"

void NetPlanner::generateNetworkOutElements(ref<Udp> udp)
{
  // <dst, <t>>
  
  ElementSpecRef pullPush =
    _conf->addElement(New refcounted< 
		      TimedPullPush >(strbuf("SendPullPush|") 
				      << _nodeID, 0));
  
  ElementSpecRef sendQueue = 
    _conf->addElement(New refcounted< Queue >(strbuf("SendQueue|") << _nodeID, 
					      QUEUESIZE));


  // <dst, <opaque>>
  ElementSpecRef marshalSend = 
    _conf->addElement(New refcounted< MarshalField >("marshal|" << 
						     _nodeID, 1));  

  // <dstAddr, <opaque>>
  ElementSpecRef routeSend =
    _conf->addElement(New refcounted< StrToSockaddr >("router|" << _nodeID, 0));

  ElementSpecRef udpSend = _conf->addElement(udp->get_tx());  
  
  _networkOut.push_back(pullPush);
  _networkOut.push_back(sendQueue);
  _networkOut.push_back(marshalSend);
  _networkOut.push_back(routeSend);
  _networkOut.push_back(udpSend);

  _conf->hookUp(pullPush, 0, sendQueue, 0);
  _conf->hookUp(sendQueue, 0, marshalSend, 0);
  _conf->hookUp(marshalSend, 0, routeSend, 0);
  _conf->hookUp(routeSend, 0, udpSend, 0);
}

void NetPlanner::generateNetworkInElements(ref<Udp> udp)
{
   // network in
  ElementSpecRef udpReceive = _conf->addElement(udp->get_rx());  
  ElementSpecRef unmarshalS =
    _conf->addElement(New refcounted< 
		      UnmarshalField >(strbuf("ReceiveUnmarshal|") 
				       << _nodeID, 1));
  ElementSpecRef unBoxS =
    _conf->addElement(New refcounted< 
		      UnboxField >(strbuf("ReceiveUnBox|") << _nodeID, 1));

  ElementSpecRef receiveBufferQueue = 
    _conf->addElement(New refcounted< Queue >(strbuf("ReceiveQueue|") << 
					      _nodeID, QUEUESIZE));
  ElementSpecRef receiveQueuePullPush = 
    _conf->addElement(New refcounted<
		      TimedPullPush>("ReceiveQueuePullPush", 0));

  _networkIn.push_back(udpReceive);
  _networkIn.push_back(unmarshalS);
  _networkIn.push_back(unBoxS);
  _networkIn.push_back(receiveBufferQueue);
  _networkIn.push_back(receiveQueuePullPush);
  
  _conf->hookUp(udpReceive, 0, unmarshalS, 0);
  _conf->hookUp(unmarshalS, 0, unBoxS, 0);
  _conf->hookUp(unBoxS, 0, receiveBufferQueue, 0);
  _conf->hookUp(receiveBufferQueue, 0, receiveQueuePullPush, 0);
}

void NetPlanner::generateNetworkElements(ref<Udp> udp)
{
  // generate network in
  generateNetworkInElements(udp);

  // generate network out
  generateNetworkOutElements(udp);
}

// static allocation given all rule strands
void 
NetPlanner::registerAllRuleStrands(std::vector<RuleStrand*> ruleStrands)
{
  // for all rule strands with receive
  // form the receiver infos
  int numReceivers = 0;
  ref< vec< ValueRef > > demuxKeys = New refcounted< vec< ValueRef > >;
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    RuleStrand* rs = ruleStrands.at(k);
    if (rs->_eca_rule->_event != NULL && rs->eventType() == Parse_Event::RECV) {
      numReceivers++;
      ReceiverInfoMap::iterator iterator 
	= _receiverInfo.find(ruleStrands.at(k)->eventFunctorName());
      if (iterator == _receiverInfo.end()) {
	ReceiverInfo* ri = New ReceiverInfo(rs->eventFunctorName());      
	_receiverInfo.insert(std::make_pair(ri->_tableName, ri));	
	ri->_receivers.push_back(rs->getEventElement());
	numReceivers++;
	demuxKeys->push_back(Val_Str::mk(ri->_tableName));
      } else {
	iterator->second->_receivers.push_back(rs->getEventElement());      
      }
    }
  }

  ElementSpecRef demuxReceive
    = _conf->addElement(New refcounted< Demux >("receiveDemux", demuxKeys));

  // create duplicators, hookup to rules
  ReceiverInfoMap::iterator iterator;
  for (iterator = _receiverInfo.begin(); iterator != _receiverInfo.end(); 
       iterator++) {  
    ReceiverInfo* ri = iterator->second;
    if (ri->_receivers.size() > 1) {
      // require a duplicator
      ElementSpecRef duplicator = 
	_conf->addElement(New refcounted< 
			  DuplicateConservative 
			  >(strbuf("DuplicateConservative|") 
			    << ri->_tableName << "|" << _nodeID, 
			    ri->_receivers.size()));
      
      for (unsigned k = 0; k < ri->_receivers.size(); k++) {
	_conf->hookUp(duplicator, k, ri->_receivers.at(k), 0);
      }
      ri->_duplicator = duplicator;
    }
  }

  // create demux  
  _conf->hookUp(_networkIn.at(_networkIn.size()-1), 0, demuxReceive, 0);
  _networkIn.push_back(demuxReceive);
   
  // hookup demux to duplicators or listeners
  for (unsigned k = 0; k < demuxKeys->size(); k++) {
    str key = Val_Str::cast((*demuxKeys)[k]);    
    ReceiverInfoMap::iterator iterator = _receiverInfo.find(key);
    ReceiverInfo* ri = iterator->second;
    
    if (ri->_duplicator == NULL) { 
      _conf->hookUp(demuxReceive, k, ri->_receivers.at(0), 0);      
      continue; 
    }
    ElementSpecRef bufferQueue = 
      _conf->addElement(New refcounted< Queue >(strbuf("DemuxDuplicatorQueue|") << 
						_nodeID << "|" << ri->_tableName, 
						QUEUESIZE));   
    ElementSpecRef pullPush = 
      _conf->addElement(New refcounted<
			TimedPullPush>(strbuf("DemuxDuplicatorQueuePullPush|") 
				       << _nodeID << "|" << ri->_tableName, 0));
    
    _conf->hookUp(demuxReceive, k, bufferQueue, 0);
    _conf->hookUp(bufferQueue, 0, pullPush, 0);
    _conf->hookUp(pullPush, 0, ri->_duplicator, 0);
  }   

  ElementSpecRef sinkS 
    = _conf->addElement(New refcounted< Discard >("DemuxDiscard"));
  _conf->hookUp(demuxReceive, demuxKeys->size(), sinkS, 0); 
  

  // for each rule strands with send
  // hookup with round robin
  int numSenders = 0;
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    if (ruleStrands.at(k)->actionType() == Parse_Action::SEND) {
      numSenders++;
    }
  }

  // create round robin
  ElementSpecRef roundRobin =
    _conf->addElement(New refcounted< RoundRobin >("roundRobinSender|" 
						   << _nodeID, 
						   numSenders)); 
  
  _conf->hookUp(roundRobin, 0, _networkOut.at(0), 0);
  _networkOut.insert(_networkOut.begin(), roundRobin);
  
  // hookup rule strands
  int counter = 0;
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    if (ruleStrands.at(k)->actionType() == Parse_Action::SEND) {
      _senders.push_back(ruleStrands.at(k)->getActionElement());
      _conf->hookUp(ruleStrands.at(k)->getActionElement(), 
		    0, roundRobin, counter++);
    }
  }
}

str NetPlanner::ReceiverInfo::toString()
{
  strbuf b;
  b << " Receiver Info: " << _tableName << " duplicate " << _receivers.size() << "\n";
  if (_duplicator != NULL) {
    b << "   Duplicator: " << _duplicator->toString() << "\n";
  }
  for (unsigned k = 0; k < _receivers.size(); k++) {
    b << "    " << _receivers.at(k)->toString() << "\n";
  }
  return str(b);
}

str NetPlanner::toString()
{
  strbuf b;
  b << "Network In:\n";
  for (unsigned k = 0; k < _networkIn.size(); k++) {
    b << " " << _networkIn.at(k)->toString() << "\n";
  }

  b << "Network Receivers:\n";
  ReceiverInfoMap::iterator _iterator;
  for (_iterator = _receiverInfo.begin(); 
       _iterator != _receiverInfo.end(); 
       _iterator++) {
    b << " " << _iterator->second->toString();
  }



  b << "Network Out:\n";
  for (unsigned k = 0; k < _networkOut.size(); k++) {
    b << " " << _networkOut.at(k)->toString() << "\n";
  }

  b << "Network Senders :\n";
  for (unsigned k = 0; k < _senders.size(); k++) {
    b << " " << _senders.at(k)->toString() << "\n";
  }
  return str(b);

}
