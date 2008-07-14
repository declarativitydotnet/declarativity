/*
 * @(#)$Id: plumber.h 1243 2007-07-16 19:05:00Z maniatis $
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#ifndef __DATAFLOWEDIT_H__
#define __DATAFLOWEDIT_H__

#include "dataflow.h"


class DataflowEdit : public Dataflow {
public:
  /** Adds *new* elements to the dataflow graph */
  ElementSpecPtr addElement(ElementPtr);

  /** Add hookup to this dataflow */
  void hookUp(ElementSpecPtr src, unsigned src_port,
              ElementSpecPtr dst, unsigned dst_port);

  /** Disconnect the element output port */
  void disconnect_output(ElementSpecPtr e, int port);
  void disconnect_output(ElementSpecPtr e, ValuePtr portKey);

  /** Disconnect the element input port */
  void disconnect_input(ElementSpecPtr e, int port);
  void disconnect_input(ElementSpecPtr e, ValuePtr portKey);

private:
  friend class Plumber;
  /** Plumber is the factory class for DataflowEdit.
      See method Plumber::edit(name) */
  DataflowEdit(DataflowPtr dpt);

  /** Completely overrides Dataflow method to perform
      connection setup using only the hookups added to
      this edit. That is, prior hookups are not reapllied */
  void set_connections();

  /** Remove the element spec from this dataflow */
  void remove(ElementSpecPtr);

  /** Adds all new elements and hookups to the dataflow
      before actual validation */
  int validate();

  /** As new hookups come in old hookups get invalidated.
      This bad boy searches and destroys old hookups. */
  void remove_old_hookups(ElementSpec::HookupPtr);

  /** Segregates new elements and hookups from old ones */
  std::vector<ElementSpecPtr>         new_elements_;
  std::vector<ElementSpec::HookupPtr> new_hookups_;
};

typedef boost::shared_ptr< DataflowEdit > DataflowEditPtr;

#endif
