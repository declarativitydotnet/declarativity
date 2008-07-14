// -*- c-basic-offset: 2; related-file-name: "elementLoader.C" -*-
/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A tokenizing element.
 *
 */

#ifndef _ELEMENTLOADER_H_
#define _ELEMENTLOADER_H_

class ElementLoader
{
public:
  /** Load all known elements */
  static void
  loadElements();
};

#endif
