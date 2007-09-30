/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: the set type.
 * 
 */
 
#include "value.h"
#include "set.h"
#include <assert.h>

Set::Set(ValuePtr v) 
{
  vpl.insert(v);
}

SetPtr Set::clone() const
{
  SetPtr l = Set::mk();
  ValPtrSet::const_iterator setp = vpl.begin();
  while(setp != vpl.end())
    l->insert(*setp++);
  return l; 
}

int Set::member(ValuePtr val) const
{
  ValPtrSet::const_iterator setp = vpl.find(val);
  //vpl.begin();
  return(setp != vpl.end());
}


ValPtrSet::const_iterator Set::begin() 
{
  return vpl.begin();
}

ValPtrSet::const_iterator Set::end() 
{
  return vpl.end();
}

SetPtr Set::intersect(SetPtr l) const
{  
  // Since copying sets requires them to be sorted, but Boost 
  // smart pointers' < relation is pretty meaningless. To work 
  // around this (and maintain immutability) we'll extract STL sets
  // from the intersected sets and work with those sets. --ACR
   
  ValPtrSet::const_iterator iter1 = vpl.begin();
  ValPtrSet::const_iterator iter2 = l->vpl.begin();   
   
  ValPtrSet::const_iterator end1 = vpl.end();
  ValPtrSet::const_iterator end2 = l->vpl.end();   
   
  SetPtr output = Set::mk();

  set_intersection(iter1, end1, iter2, end2, inserter(output->vpl, output->vpl.begin()), ltSet());

  return output;
}



SetPtr Set::difference(SetPtr l) const
{  
  // Since copying sets requires them to be sorted, but Boost 
  // smart pointers' < relation is pretty meaningless. To work 
  // around this (and maintain immutability) we'll extract STL sets
  // from the intersected sets and work with those sets. --ACR
   
  ValPtrSet::const_iterator iter1 = vpl.begin();
  ValPtrSet::const_iterator iter2 = l->vpl.begin();   
   
  ValPtrSet::const_iterator end1 = vpl.end();
  ValPtrSet::const_iterator end2 = l->vpl.end();   
   
  SetPtr output = Set::mk();

  set_difference(iter1, end1, iter2, end2, inserter(output->vpl, output->vpl.begin()), ltSet());

  return output;
}

SetPtr Set::symmetricDiff(SetPtr l) const
{  
  // Since copying sets requires them to be sorted, but Boost 
  // smart pointers' < relation is pretty meaningless. To work 
  // around this (and maintain immutability) we'll extract STL sets
  // from the intersected sets and work with those sets. --ACR
   
  ValPtrSet::const_iterator iter1 = vpl.begin();
  ValPtrSet::const_iterator iter2 = l->vpl.begin();   
   
  ValPtrSet::const_iterator end1 = vpl.end();
  ValPtrSet::const_iterator end2 = l->vpl.end();   
   
  SetPtr output = Set::mk();

  set_symmetric_difference(iter1, end1, iter2, end2, inserter(output->vpl, output->vpl.begin()), ltSet());

  return output;
}

void Set::insert(ValuePtr val)
{
  vpl.insert(val);
}

bool Set::subset(SetPtr l) const 
{
  ValPtrSet::const_iterator iter1 = vpl.begin();
  ValPtrSet::const_iterator iter2 = l->vpl.begin();   
   
  ValPtrSet::const_iterator end1 = vpl.end();
  ValPtrSet::const_iterator end2 = l->vpl.end();   
  return includes(iter1, end1, iter2, end2, ltSet()) ;
}

bool Set::propersubset(SetPtr l) const 
{
  return (subset(l) && (vpl.size() > l->vpl.size()));
}

SetPtr Set::setunion(SetPtr l) const
{   
  SetPtr output = Set::mk();

  ValPtrSet::const_iterator iter1 = vpl.begin();
  ValPtrSet::const_iterator iter2 = l->vpl.begin();   
   
  ValPtrSet::const_iterator end1 = vpl.end();
  ValPtrSet::const_iterator end2 = l->vpl.end();   

  set_union(iter1, end1, iter2, end2, inserter(output->vpl, output->vpl.begin()), ltSet());
   
  return output;
}

int Set::compareTo(SetPtr other) const
{

  ValPtrSet mySet = vpl;
  
   ValPtrSet::const_iterator i1 = mySet.begin();
   ValPtrSet::const_iterator e1 = mySet.end();
   ValPtrSet::const_iterator i2 = other->begin();
   ValPtrSet::const_iterator e2 = other->end();
   
   while(i1 != e1 && i2 != e2) {
      if((*i1)->compareTo(*i2) != 0) {
         return (*i1)->compareTo(*i2);
      } else {
         i1++;
         i2++;
      }
   }
   
   bool l1Done = (i1 == e1);
   bool l2Done = (i2 == e2);
   
   if(l1Done && !l2Done) {
      // L1 is a prefix of L2
      return -1;
   } else if (l2Done && !l1Done) {
      // L2 is a prefix of L1
      return 1;
   } else {
      return 0;
   }
}

string Set::toString() const 
{
  ostringstream sb;
   
  sb << "{";
   
  ValPtrSet::const_iterator iter = vpl.begin();
  ValPtrSet::const_iterator end = vpl.end();
  if(iter != end)
  {
    sb << (*iter)->toString();
    iter++;
  }

  while(iter != end) {
    sb << ", ";
    sb << (*iter)->toString();
    iter++;
  }
   
  sb << "}";
   
  return sb.str();
}

// Marshal a Set into an XDR
void Set::xdr_marshal(XDR *x) 
{
  uint32_t size = vpl.size();
  u_int32_t uintsize = (u_int32_t) size;
   
  xdr_uint32_t(x, &uintsize);
   
  ValPtrSet::const_iterator iter = vpl.begin();
  ValPtrSet::const_iterator end = vpl.end();

  while(iter != end) {
    (*iter)->xdr_marshal(x);
    iter++;
  }
}

// Unmarshal an XDR into a Set
SetPtr Set::xdr_unmarshal(XDR *x)
{
  SetPtr lp = Set::mk();
   
  u_int32_t uintsize;
   
  xdr_uint32_t(x, &uintsize);
   
  uint32_t size = uintsize;
   
  for(uint32_t i = 0;
      i < size;
      i++) {
    lp->insert(Value::xdr_unmarshal(x));
  }
   
  return lp;
}



