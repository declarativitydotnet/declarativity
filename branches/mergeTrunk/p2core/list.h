/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Pel's list type
 */
 
#ifndef __LIST_H__
#define __LIST_H__


#include <boost/shared_ptr.hpp>
#include <list>
#include <map>
#include "value.h"
#include <utility>

class List;
typedef boost::shared_ptr<List> ListPtr;
typedef std::list< ValuePtr > ValPtrList;
typedef std::map< std::string, std::pair< ValuePtr, int > > ValMap;
class List {

public:
   // Factory
   static ListPtr mk() { ListPtr l(new List()); return l; };
   static ListPtr mk(ValueList *vl) { 
     ListPtr l(new List()); 
     for (ValueList::iterator i = vl->begin(); i != vl->end(); i++)
       l->append(*i);
     return l; 
   };

   ListPtr clone() const;

   List() : vpl() {};
   
   // Constructs a list consisting of a single element.
   List(ValuePtr v);

   // Constructs a list containing a copy of the passed list.
   List(ValPtrList list) : vpl(list) {};

   // Is the passed ValuePtr a member of the list?
   int member(ValuePtr val) const;

   // Intersect this list with another list and return the result. 
   // Result is ordered, and duplicates are not necessarily preserved. 
   // This is the intersection style described in Steele's Common Lisp.
   ListPtr intersect(ListPtr l) const;

   // Intersect this list with another list and return the result.
   // This intersection treats the lists as multisets. 
   
   ListPtr multiset_intersect(ListPtr l) const;
   
   uint32_t size() const { return vpl.size(); };
   
   // Appends a value to a list.
   void append(ValuePtr val);

   void append(ListPtr list);

   // Prepends a value to a list.
   void prepend(ValuePtr val);
   
   // Concatenates two lists together. This is the functional 
   // equivalent of Lisp-style cons. Ordering in the new list is 
   // assumed to be (this, L)
   
   ListPtr concat(ListPtr L) const;
   
   ValPtrList::const_iterator begin();
   
   ValPtrList::const_iterator end();
      
   string toString() const;
   
   int compareTo(ListPtr) const;
   
   void xdr_marshal( XDR *xdrs );
   
   static ListPtr xdr_unmarshal( XDR *xdrs );

   void sort();

   ValuePtr front() { return vpl.front(); }
   ValuePtr back() { return vpl.back(); }

   ValuePtr at(uint32_t pos) const;

   void pop_front() { vpl.pop_front(); }
   void pop_back() { vpl.pop_back(); }
      
private:
   ValPtrList vpl;
};



#endif /* __LIST_H__ */
