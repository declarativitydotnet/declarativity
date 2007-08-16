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

bool valueSortCriterion(const ValuePtr& v1, const ValuePtr& v2)
{
  return (v1->compareTo(v2) < 0) ? true : false;
}

Set::Set(ValuePtr v) 
{
  vpl.push_back(v);
}

SetPtr Set::clone() const
{
  SetPtr l = Set::mk();
  ValPtrSet::const_iterator setp = vpl.begin();
  while(setp != vpl.end())
    l->append(*setp++);
  return l; 
}

int Set::member(ValuePtr val) const
{
  ValPtrSet::const_iterator setp = vpl.begin();
   
  while(setp != vpl.end()) {
    if((*setp)->compareTo(val) == 0) {
      return 1;
    }
      
    setp++;
  }
   
  return 0;
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
   
  ValPtrSet l1;
  ValPtrSet l2;
   
  ValPtrSet::const_iterator iter1 = vpl.begin();
  ValPtrSet::const_iterator iter2 = l->begin();   
   
  ValPtrSet::const_iterator end1 = vpl.end();
  ValPtrSet::const_iterator end2 = l->end();   
   
  while(iter1 != end1) {
    l1.push_back(*iter1);
    iter1++;
  }
   
  while(iter2 != end2) {
    l2.push_back(*iter2);
    iter2++;
  }
   
  l1.sort(valueSortCriterion);
  l2.sort(valueSortCriterion);
   
  iter1 = l1.begin();
  iter2 = l2.begin();
   
  end1 = l1.end();
  end2 = l2.end();

  SetPtr output = Set::mk();

  while(iter1 != end1 && iter2 != end2) {
    ValuePtr i1 = *iter1;
    ValuePtr i2 = *iter2;
      
    int compareResult = i1->compareTo(i2);
      
    if(compareResult == 0) {
      output->append(*iter1);
      iter1++;
      iter2++;
    } else if(compareResult < 0) {
      iter1++;
    } else {
      iter2++;
    }
  }
   
  return output;
}


void Set::append(ValuePtr val)
{
  vpl.push_back(val);
}

void Set::prepend(ValuePtr val)
{
  vpl.push_front(val);
}

SetPtr Set::concat(SetPtr L) const
{   
  SetPtr out = Set::mk();
   
  ValPtrSet::const_iterator i = vpl.begin();
  ValPtrSet::const_iterator end = vpl.end();
   
  while(i != end) {
    out->append(*i);
    i++;
  }
   
  i = L->begin();
  end = L->end();  
   
  while(i != end) {
    out->append(*i);
    i++;
  }
   
  return out;
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
   
  sb << "(";
   
  ValPtrSet::const_iterator iter = vpl.begin();
  ValPtrSet::const_iterator end = vpl.end();
  ValPtrSet::const_iterator almost_end = vpl.end();
  almost_end--;
   
  while(iter != end) {
    sb << (*iter)->toString();
      
    if(iter != almost_end) {
      sb << ", ";
    }
    iter++;
  }
   
  sb << ")";
   
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
    lp->append(Value::xdr_unmarshal(x));
  }
   
  return lp;
}


//sort
void Set::sort()
{
  vpl.sort(Value::Comparator());
}
