/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Iterator object template
 *
 */

#ifndef __ITERATOROBJ_H__
#define __ITERATOROBJ_H__

template < typename _Index, typename _FlatIndex >
Table::IteratorObj< _Index, _FlatIndex >::IteratorObj(_Index * index,
                                                      ValuePtr key)
  : _index(),
    _key(key),
    _iter(_index.find(_key))
{
  assert(index != NULL);
  // copying the index locally!!!
  for (IndexIterator i = index->begin();
       i != index->end();
       i++) {
    _index.insert(std::make_pair(i->first, i->second->t));
  }
  _iter = _index.find(_key);
}

template < typename _Index, typename _FlatIndex >
TuplePtr
Table::IteratorObj< _Index, _FlatIndex >::next()
{
  if (_iter == _index.end()) {
    // We've run out of elements, period.
    return TuplePtr();
  } else {
    ValuePtr foundKey = _iter->first;
    if (foundKey->compareTo(_key) != 0) {
      // We've gone past the end of this key
      return TuplePtr();
    } else {
      return (_iter++)->second;
    }
  }
}

template < typename _Index, typename _FlatIndex >
bool
Table::IteratorObj< _Index, _FlatIndex >::done()
{
  if (_iter == _index.end()) {
    return true;
  } else {
    if (_iter->first->compareTo(_key) != 0) {
      return true;
    } else {
      return false;
    }
  }
}

#endif /* __ITERATOROBJ_H__ */
