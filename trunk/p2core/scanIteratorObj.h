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
 * DESCRIPTION: Scan iterator object template
 *
 */

#ifndef __SCANITERATOROBJ_H__
#define __SCANITERATOROBJ_H__


template < typename _Index >
Table::ScanIteratorObj< _Index >::ScanIteratorObj(_Index * index)
  : _index(index),
    _iter(_index->begin())
{
}

template < typename _Index >
TuplePtr
Table::ScanIteratorObj< _Index >::next()
{
  if (_iter == _index->end()) {
    // We've run out of elements, period.
    return NULL;
  } else {
    return (_iter++)->second->t;
  }
}

template < typename _Index >
bool
Table::ScanIteratorObj< _Index >::done()
{
  return (_iter == _index->end());
}

template < typename _Index >
void
Table::ScanIteratorObj< _Index >::reset()
{
  _iter = _index->begin();
}

#endif /* __SCANITERATOROBJ_H__ */
