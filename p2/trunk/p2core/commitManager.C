#include "commitManager.h"

CommitManager::CommitManager(){}

void CommitManager::addAction(ActionPtr anAct)
{
  mActionList.push_back(anAct);
}

void CommitManager::removeAction(ActionPtr anAct)
{
  mActionList.remove(anAct);
}

void CommitManager::commit()
{
  ActionList::iterator it = mActionList.begin();
  for(;it!=mActionList.end();it++){
    (*it)->commit();
  }
}

void CommitManager::commit(ActionPtr anAction)
{
	anAction->commit();
}

void CommitManager::abort()
{
  ActionList::iterator it = mActionList.begin();
  for(;it!=mActionList.end();it++){
    (*it)->abort();
  }
}

void CommitManager::abort(ActionPtr anAction)
{
	anAction->abort();
}


void CommitManager::Action::addTuple(TuplePtr t)
{
  _buffer.insert(t);
}

CommitManager::Action::~Action()
{
}

void CommitManager::Action::removeTuple(TuplePtr t)
{
  _buffer.erase(_buffer.find(t));
}

