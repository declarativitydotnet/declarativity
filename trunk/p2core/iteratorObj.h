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
 * DESCRIPTION: Iterator object template
 *
 */

#ifndef __ITERATOROBJ_H__
#define __ITERATOROBJ_H__

template < typename _Index >
Table::IteratorObj< _Index >::IteratorObj(_Index * index,
                                          ValueRef key)
  : _index(index),
    _key(key),
    _iter(_index->find(_key))
{
  assert(_index != NULL);
}

template < typename _Index >
TuplePtr
Table::IteratorObj< _Index >::next()
{
  if (_iter == _index->end()) {
    // We've run out of elements, period.
    return NULL;
  } else {
    ValueRef foundKey = _iter->first;
    if (foundKey->compareTo(_key) != 0) {
      // We've gone past the end of this key
      return NULL;
    } else {
      return (_iter++)->second->t;
    }
  }
}

template < typename _Index >
bool
Table::IteratorObj< _Index >::done()
{
  if (_iter == _index->end()) {
    return true;
  } else {
    return (_iter->first->compareTo(_key) != 0);
  }
}

#endif /* __ITERATOROBJ_H__ */
