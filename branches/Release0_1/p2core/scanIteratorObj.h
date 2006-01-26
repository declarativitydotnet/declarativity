/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Scan iterator object template
 *
 */

#ifndef __SCANITERATOROBJ_H__
#define __SCANITERATOROBJ_H__


template < typename _Index, typename _FlatIndex >
Table::ScanIteratorObj< _Index, _FlatIndex >::ScanIteratorObj(_Index * index)
  : _index(),
    _iter(_index.begin())
{
  // copying the index locally!!!
  for (IndexIterator i = index->begin();
       i != index->end();
       i++) {
    _index.insert(std::make_pair(i->first, i->second->t));
  }
  _iter = _index.begin();
}

template < typename _Index, typename _FlatIndex >
TuplePtr
Table::ScanIteratorObj< _Index, _FlatIndex >::next()
{
  if (_iter == _index.end()) {
    // We've run out of elements, period.
    return TuplePtr();
  } else {
    FlatIndexIterator current = _iter;
    _iter++;
    return current->second;
  }
}

template < typename _Index, typename _FlatIndex >
bool
Table::ScanIteratorObj< _Index, _FlatIndex >::done()
{
  return (_iter == _index.end());
}

template < typename _Index, typename _FlatIndex >
void
Table::ScanIteratorObj< _Index, _FlatIndex >::addListener(Table::Listener listenerCallback)
{
  _listeners.push_back(listenerCallback);
}

template < typename _Index, typename _FlatIndex >
void
Table::ScanIteratorObj< _Index, _FlatIndex >::update(TuplePtr t)
{
  for (size_t i = 0;
       i < _listeners.size();
       i++) {
    Listener listener = _listeners[i];
    listener(t);
  }
}




#endif /* __SCANITERATOROBJ_H__ */
