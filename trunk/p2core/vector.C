// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click Vector base class by Eddie Kohler
 * 
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2004 Regents of the University of California
 * Copyright (c) 2004 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Vector class used by the router infrastructure
 * 
 * Note: Eventually we should consolidate the two vector implementations
 * in the P2 core (one from Click and one from the SFS base).  Arguably,
 * sticking with the Click vector makes more sense, since that
 * implementation is more deeply intertwined with the scheduling
 * infrastructure.
 */

#ifndef __VECTOR_CC__
#define __VECTOR_CC__

#include <vector.h>

template <class T>
Vector<T>::Vector(const Vector<T> &o)
  : _l(0), _n(0), _capacity(0)
{
  *this = o;
}

template <class T>
Vector<T>::~Vector()
{
  for (int i = 0; i < _n; i++)
    _l[i].~T();
  delete[] (unsigned char *)_l;
}

template <class T> Vector<T> &
Vector<T>::operator=(const Vector<T> &o)
{
  if (&o != this) {
    for (int i = 0; i < _n; i++)
      _l[i].~T();
    _n = 0;
    if (reserve(o._n)) {
      _n = o._n;
      for (int i = 0; i < _n; i++)
        new(velt(i)) T(o._l[i]);
    }
  }
  return *this;
}

template <class T> Vector<T> &
Vector<T>::assign(int n, const T &e)
{
  resize(0, e);
  resize(n, e);
  return *this;
}

template <class T> typename Vector<T>::iterator
Vector<T>::insert(iterator i, const T& e)
{
  assert(i >= begin() && i <= end());
  int pos = i - begin();
  if (_n < _capacity || reserve(-1)) {
    for (iterator j = end() - 1; j >= begin() + pos; j--) {
      new((void*) (j+1)) T(*j);
      j->~T();
    }
    new(velt(pos)) T(e);
    _n++;
  }
  return begin() + pos;
}

template <class T> typename Vector<T>::iterator
Vector<T>::erase(iterator a, iterator b)
{
  if (b > a) {
    assert(a >= begin() && b <= end());
    iterator i = a, j = b;
    for (; j < end(); i++, j++) {
      i->~T();
      new((void*) i) T(*j);
    }
    for (; i < end(); i++)
      i->~T();
    _n -= b - a;
    return a;
  } else
    return b;
}

template <class T> bool
Vector<T>::reserve(int want)
{
  if (want < 0)
    want = _capacity > 0 ? _capacity * 2 : 4;
  if (want <= _capacity)
    return true;
  
  T *new_l = (T *)new unsigned char[sizeof(T) * want];
  if (!new_l)
    return false;
  
  for (int i = 0; i < _n; i++) {
    new(velt(new_l, i)) T(_l[i]);
    _l[i].~T();
  }
  delete[] (unsigned char *)_l;
  
  _l = new_l;
  _capacity = want;
  return true;
}

template <class T> void
Vector<T>::resize(int nn, const T &e)
{
  if (nn <= _capacity || reserve(nn)) {
    for (int i = nn; i < _n; i++)
      _l[i].~T();
    for (int i = _n; i < nn; i++)
      new(velt(i)) T(e);
    _n = nn;
  }
}

template <class T> void
Vector<T>::swap(Vector<T> &o)
{
  T *l = _l;
  int n = _n;
  int cap = _capacity;
  _l = o._l;
  _n = o._n;
  _capacity = o._capacity;
  o._l = l;
  o._n = n;
  o._capacity = cap;
}

#endif
