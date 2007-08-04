/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Network planner code
 *
 */

#include "netPlanner.h"
#include "demuxConservative.h"

void
NetPlanner::generateNetworkOutElements(boost::shared_ptr<Udp> udp)
{
  // <dst, <t>>
  
  ElementSpecPtr pullPush =
    _conf->addElement(ElementPtr(new TimedPullPush("SendPullPush", 0)));
  
  ElementSpecPtr sendQueue = 
    _conf->addElement(ElementPtr(new Queue("SendQueue", QUEUESIZE)));

  // If wrap around, we unbox
  std::vector< ValuePtr > wrapAroundDemuxKeys;
  wrapAroundDemuxKeys.push_back(ValuePtr(new Val_Str(_nodeID)));
  _wrapAroundSendDemux = 
    _conf->addElement(ElementPtr(new DemuxConservative("wrapAroundSendDemux",
                                                       wrapAroundDemuxKeys,
                                                       0)));  

  // <dst, <opaque>>
  ElementSpecPtr marshalSend = 
    _conf->addElement(ElementPtr(new MarshalField("marshal!" + _nodeID, 1)));  

  // <dstAddr, <opaque>>
  ElementSpecPtr routeSend =
    _conf->addElement(ElementPtr(new StrToSockaddr("plumber!" + _nodeID, 0)));

  ElementSpecPtr udpSend = _conf->addElement(udp->get_tx());  
  
  _networkOut.push_back(pullPush);
  _networkOut.push_back(sendQueue);
  _networkOut.push_back(marshalSend);
  _networkOut.push_back(routeSend);
  _networkOut.push_back(udpSend);
  
  _conf->hookUp(pullPush, 0, _wrapAroundSendDemux, 0);
  _conf->hookUp(_wrapAroundSendDemux, 1, sendQueue, 0);
  _conf->hookUp(sendQueue, 0, marshalSend, 0);
  _conf->hookUp(marshalSend, 0, routeSend, 0);
  _conf->hookUp(routeSend, 0, udpSend, 0);
}


void
NetPlanner::generateNetworkInElements(boost::shared_ptr<Udp> udp)
{
   // network in
  ElementSpecPtr udpReceive = _conf->addElement(udp->get_rx());  
  ElementSpecPtr unmarshalS =
    _conf->addElement(ElementPtr(new UnmarshalField("ReceiveUnmarshal", 1)));
  ElementSpecPtr bufferQueue = 
    _conf->addElement(ElementPtr(new Queue("ReceiveQueue", QUEUESIZE)));

  ElementSpecPtr bufferQueuePullPush = 
    _conf->addElement(ElementPtr(new TimedPullPush("ReceiveQueuePullPush", 0)));

  ElementSpecPtr receiveMux = _conf->addElement(ElementPtr(new Mux("wrapAroundSendMux:", 2)));


  ElementSpecPtr unBoxS =
    _conf->addElement(ElementPtr(new UnboxField("ReceiveUnBox", 1)));



  _networkIn.push_back(udpReceive);
  _networkIn.push_back(unmarshalS);
  _networkIn.push_back(receiveMux);
  _networkIn.push_back(unBoxS);
  _networkIn.push_back(bufferQueue);
  _networkIn.push_back(bufferQueuePullPush);

  // From outside
  _conf->hookUp(udpReceive, 0, unmarshalS, 0);
  _conf->hookUp(unmarshalS, 0, receiveMux, 1);  

  // From wraparound
  _conf->hookUp(_wrapAroundSendDemux, 0, receiveMux, 0);


  // Link mux to receive demux
  _conf->hookUp(receiveMux, 0, unBoxS, 0);
  _conf->hookUp(unBoxS, 0, bufferQueue, 0);
  _conf->hookUp(bufferQueue, 0, bufferQueuePullPush, 0);
}

void NetPlanner::generateNetworkElements(boost::shared_ptr<Udp> udp)
{
  // generate network out
  generateNetworkOutElements(udp);

  // generate network in
  generateNetworkInElements(udp);
}

// static allocation given all rule strands
void 
NetPlanner::registerAllRuleStrands(std::vector<RuleStrand*> ruleStrands,
                                   std::vector<StageStrand*> stageStrands)
{
  // for all rule strands with receive, form the receiver infos
  int numReceivers = 0;
  std::vector< ValuePtr > demuxKeys;
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    RuleStrand* rs = ruleStrands.at(k);
    if (rs->getRule()->_event != NULL
        && rs->eventType() == Parse_Event::P2_RECV) {
      numReceivers++;
      ReceiverInfoMap::iterator iterator =
        _receiverInfo.find(ruleStrands.at(k)->eventFunctorName());
      if (iterator == _receiverInfo.end()) {
	ReceiverInfo* ri = new ReceiverInfo(rs->eventFunctorName());      
	_receiverInfo.insert(std::make_pair(ri->_tableName, ri));	
	ri->_receivers.push_back(rs->getFirstElement());
	numReceivers++;
	demuxKeys.push_back(Val_Str::mk(ri->_tableName));
      } else {
	iterator->second->_receivers.push_back(rs->getFirstElement());      
      }
    }
  }


  // for all stage strands, form the receiver infos
  for (unsigned k = 0; k < stageStrands.size(); k++) {
    StageStrand* ss = stageStrands.at(k);
    numReceivers++;
    ReceiverInfoMap::iterator iterator =
      _receiverInfo.find(ss->inputName());
    if (iterator == _receiverInfo.end()) {
      // We don't have such a demux key yet. Create that receiver.
      ReceiverInfo* ri = new ReceiverInfo(ss->inputName());      
      _receiverInfo.insert(std::make_pair(ri->_tableName, ri));	
      ri->_receivers.push_back(ss->getFirstElement());
      numReceivers++;
      demuxKeys.push_back(Val_Str::mk(ri->_tableName));
    } else {
      iterator->second->_receivers.push_back(ss->getFirstElement());      
    }
  }
  


  ElementSpecPtr demuxReceive =
    _conf->addElement(ElementPtr(new DemuxConservative("receiveDemux",
                                                       demuxKeys)));

  // create duplicators, hookup to rules
  ReceiverInfoMap::iterator iterator;
  for (iterator = _receiverInfo.begin(); iterator != _receiverInfo.end(); 
       iterator++) {  
    ReceiverInfo* ri = iterator->second;
    if (ri->_receivers.size() > 1) {
      // require a duplicator
      ElementSpecPtr duplicator = 
	_conf->addElement(ElementPtr(new DuplicateConservative("DuplicateConservative!" 
			    + ri->_tableName, ri->_receivers.size())));
      
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
  for (unsigned k = 0; k < demuxKeys.size(); k++) {
    string key = Val_Str::cast((demuxKeys)[k]);    
    ReceiverInfoMap::iterator iterator = _receiverInfo.find(key);
    ReceiverInfo* ri = iterator->second;
    
    if (ri->_duplicator == NULL) { 
      _conf->hookUp(demuxReceive, k, ri->_receivers.at(0), 0);      
      continue; 
    }
    ElementSpecPtr bufferQueue = 
      _conf->addElement(ElementPtr(new Queue("DemuxDuplicatorQueue!" +  
                                             ri->_tableName, QUEUESIZE)));   
    ElementSpecPtr pullPush = 
      _conf->addElement(ElementPtr(new TimedPullPush("DemuxDuplicatorQueuePullPush!" + 
				                     ri->_tableName, 0)));
    
    _conf->hookUp(demuxReceive, k, bufferQueue, 0);
    _conf->hookUp(bufferQueue, 0, pullPush, 0);
    _conf->hookUp(pullPush, 0, ri->_duplicator, 0);
  }   

  ElementSpecPtr sinkS 
    = _conf->addElement(ElementPtr(new Discard("DemuxDiscard")));
  _conf->hookUp(demuxReceive, demuxKeys.size(), sinkS, 0); 
  

  // for each rule strands with send
  // hookup with round robin
  int numSenders = 0;
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    if (ruleStrands.at(k)->actionType() == Parse_Action::P2_SEND) {
      numSenders++;
    }
  }
  numSenders += stageStrands.size();

  // create round robin
  ElementSpecPtr roundRobin =
    _conf->addElement(ElementPtr(new RoundRobin("roundRobinSender!" + _nodeID, 
						   numSenders))); 
  
  _conf->hookUp(roundRobin, 0, _networkOut.at(0), 0);
  _networkOut.insert(_networkOut.begin(), roundRobin);
  
  // hookup rule strands
  int counter = 0;
  for (unsigned k = 0; k < ruleStrands.size(); k++) {
    if (ruleStrands.at(k)->actionType() == Parse_Action::P2_SEND) {
      _senders.push_back(ruleStrands.at(k)->getActionElement());
      _conf->hookUp(ruleStrands.at(k)->getActionElement(), 
		    0, roundRobin, counter++);
    }
  }
  // hookup stage strands
  for (unsigned k = 0; k < stageStrands.size(); k++) {
    _senders.push_back(stageStrands.at(k)->getLastElement());
    _conf->hookUp(stageStrands.at(k)->getLastElement(), 
                  0, roundRobin, counter++);
  }
}

string NetPlanner::ReceiverInfo::toString()
{
  ostringstream b;
  b << " Receiver Info: " << _tableName << " duplicate " << _receivers.size() << "\n";
  if (_duplicator != NULL) {
    b << "   Duplicator: " << _duplicator->toString() << "\n";
  }
   for (unsigned k = 0; k < _receivers.size(); k++) {
    b << "    " << _receivers.at(k)->toString() << "\n";
  }
  return b.str();
}

string NetPlanner::toString()
{
  ostringstream b;
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
  return b.str();

}
