/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: the list type.
 * 
 */
 
#include "value.h"
#include "list.h"
#include <assert.h>

bool valueSortCriterion(const ValuePtr& v1, const ValuePtr& v2)
{
  return (v1->compareTo(v2) < 0) ? true : false;
}

List::List(ValuePtr v) 
{
  vpl.push_back(v);
}

int List::member(ValuePtr val) const
{
  ValPtrList::const_iterator listp = vpl.begin();
   
  while(listp != vpl.end()) {
    if((*listp)->compareTo(val) == 0) {
      return 1;
    }
      
    listp++;
  }
   
  return 0;
}

ValPtrList::const_iterator List::begin() 
{
  return vpl.begin();
}

ValPtrList::const_iterator List::end() 
{
  return vpl.end();
}

ListPtr List::intersect(ListPtr l) const
{  
  // Since copying lists requires them to be sorted, but Boost 
  // smart pointers' < relation is pretty meaningless. To work 
  // around this (and maintain immutability) we'll extract STL lists
  // from the intersected lists and work with those lists. --ACR
   
  ValPtrList l1;
  ValPtrList l2;
   
  ValPtrList::const_iterator iter1 = vpl.begin();
  ValPtrList::const_iterator iter2 = l->begin();   
   
  ValPtrList::const_iterator end1 = vpl.end();
  ValPtrList::const_iterator end2 = l->end();   
   
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

  ListPtr output = List::mk();

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

ListPtr List::multiset_intersect(ListPtr l) const
{
  ValMap listValues;
   
  ValPtrList::const_iterator iter = l->begin();
  ValPtrList::const_iterator end = l->end();
      
  while(iter != end) {
    ValMap::iterator item = listValues.find((*iter)->toString());
      
    if(item == listValues.end()) {
      listValues[(*iter)->toString()] = std::make_pair(*iter, 1);
    } else {
      listValues[(*iter)->toString()] = std::make_pair(*iter, 
                                                       (*item).second.second + 1);
    }
      
    iter++;
  }
   
  ListPtr output = List::mk();
   
  iter = vpl.begin();
  end = vpl.end();

  while(iter != end) {
    ValMap::iterator item = listValues.find((*iter)->toString());
      
    if(item != listValues.end() && 
       listValues[(*iter)->toString()].second != 0) 
      {
        output->append((*item).second.first);
        listValues[(*iter)->toString()] = std::make_pair(*iter, 
                                                         (*item).second.second - 1);
      }
      
    iter++;
  }
   
  return output;
}

void List::append(ValuePtr val)
{
  vpl.push_back(val);
}

ListPtr List::concat(ListPtr L) const
{   
  ListPtr out = List::mk();
   
  ValPtrList::const_iterator i = vpl.begin();
  ValPtrList::const_iterator end = vpl.end();
   
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

int List::compareTo(ListPtr other) const
{
  ValPtrList myList = vpl;
   
  int mySize = myList.size();
  int otherSize = other->size();
   
  if(mySize == otherSize) {
    ValPtrList::const_iterator mylistp = myList.begin();
    ValPtrList::const_iterator otherlistp = other->begin();
    ValPtrList::const_iterator myendp = myList.end();
    ValPtrList::const_iterator otherendp = other->end();
      
      
    while(mylistp != myendp && otherlistp != otherendp) {
      if((*mylistp)->compareTo(*otherlistp) != 0) {
        return (*mylistp)->compareTo(*otherlistp);
      }
         
      mylistp++;
      otherlistp++;
    }
      
    return 0;
  } else if(mySize < otherSize) {
    return -1;
  } else {
    return 1;
  }
}

string List::toString() const 
{
  ostringstream sb;
   
  sb << "(";
   
  ValPtrList::const_iterator iter = vpl.begin();
  ValPtrList::const_iterator end = vpl.end();
  ValPtrList::const_iterator almost_end = vpl.end();
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

// Marshal a List into an XDR
void List::xdr_marshal(XDR *x) 
{
  uint32_t size = vpl.size();
  u_int32_t uintsize = (u_int32_t) size;
   
  xdr_uint32_t(x, &uintsize);
   
  ValPtrList::const_iterator iter = vpl.begin();
  ValPtrList::const_iterator end = vpl.end();

  while(iter != end) {
    (*iter)->xdr_marshal(x);
    iter++;
  }
}

// Unmarshal an XDR into a List
ListPtr List::xdr_unmarshal(XDR *x)
{
  ListPtr lp = List::mk();
   
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
