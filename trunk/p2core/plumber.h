// -*- c-basic-offset: 2; related-file-name: "plumber.C" -*-
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
 * Loosely inspired from the Click Router class, by Eddie Kohler
 *
 * DESCRIPTION: The plumber shell providing plumbing functionality among
 * elements.
 */

#ifndef __PLUMBER_H__
#define __PLUMBER_H__

#include <inlines.h>
#include <iostream>
#include <fstream>
#include <element.h>
#include <elementSpec.h>
#include <loggerI.h>
#include <dot.h>

/** A handy dandy type for plumber references */
class Plumber;
typedef boost::shared_ptr< Plumber > PlumberPtr;

class Plumber {
public:
  class Dataflow {
    public:
      Dataflow(string name="dataflow") : name_(name) {};

      virtual ~Dataflow() {};

      /** The name of this dataflow */
      string name() const { return name_; }

      /** The string representation of each element that is part 
       *  of this dataflow */
      string toString() const;

      /** Validate this dataflow. These checks are local to
          this dataflow (they don't require info from other dataflows). */
      virtual int validate();

      /** Finalize this dataflow */
      void finalize();

      /** Add a new element to this dataflow by creating a
          new ElementSpecPtr that references the passed in element.
       */
      virtual ElementSpecPtr addElement(ElementPtr);

      /** Add hookup to this dataflow */
      virtual void
              hookUp(ElementSpecPtr src, int src_port,
                     ElementSpecPtr dst, int dst_port );

      /**
       *  All elements are local to this dataflow regardless of operation.
       *  Elements in other dataflows under INSERT and REMOVE operations
       *  will be indicated in the hookups.
       */
      std::vector< ElementSpecPtr > elements_;

      /** The hookups */
      std::vector< ElementSpec::HookupPtr > hookups_;

    protected:

      /** Are the configuration hookups refering to existing elements and
          non-negative ports? */
      int check_hookup_elements();
    
      /** Are the port numbers within the attached element's range? */
      int check_hookup_range();
    
      /** Is personality semantics correctly applied to hookups?  This only
          checks that the end-points of a hookup are consistent.  It does
          not follow flow codes through elements. */
      int check_push_and_pull();

      /** Run over all dataflow element specs and check for 
       *  hookup completeness. Completely disconnect elements from
       *  the 'installedDataflowName' Dataflow will cause an error. 
       *  Disconnected elements in other dataflows will be automatically
       *  garbage collected.
       */
      virtual int check_hookup_completeness();

      /** Are any ports multiply connected?  Are all ports attached to
        * something?  We require all ports to be attached to something
        * (exactly one something), even pull outputs and push inputs. */
      virtual int eval_hookups();

      /** Perform the actual hooking up of real elements from the specs. No
          error checking at this point.  */
      virtual void set_connections();

      /** The root name of the dataflow */
      string name_;
  };
  typedef boost::shared_ptr< Dataflow > DataflowPtr;

  class DataflowEdit : public Dataflow {
    public:

      /** Locate an element with a given string name 
       *  that exists in the dataflow being edited.
       *  Return: ElementSpecPtr to element on success or
       *          an empty ElementSpecPtr object. */
      ElementSpecPtr find(string);

      /** Adds *new* elements to the dataflow graph */
      ElementSpecPtr addElement(ElementPtr);

      /** Add hookup to this dataflow */
      void hookUp(ElementSpecPtr src, int src_port,
                  ElementSpecPtr dst, int dst_port);

    private:
      friend class Plumber;
      /** Plumber is the factory class for DataflowEdit.
          See method Plumber::new_dataflow_edit(name) */
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

  /** Create a new plumber given a configuration of constructed but not
      necessarily configured elements. */
  Plumber(LoggerI::Level loggingLevel = LoggerI::NONE);

  /** Return a reference to a new dataflow object */
  DataflowEditPtr new_dataflow_edit(string name) { 
    DataflowPtr dpt = dataflow(name);
    if (dpt == 0) 
      return DataflowEditPtr();
    return DataflowEditPtr(new DataflowEdit(dpt)); 
  }

  /** Install the dataflow in this plumber. 
   *  Return: 0 on success, -1 on failure.
   */
  int install(DataflowPtr d);

  /** Output in .dot format a representation of this dataflow */
  void toDot(string f);

  int ndataflows() const			{ return _dataflows->size(); }
  DataflowPtr dataflow(string name) {
    std::map<string, DataflowPtr>::iterator i = _dataflows->find(name);
    return (i == _dataflows->end()) ? DataflowPtr() : i->second;
  }

  // LOGGING infrastructure
  
  /** The plumber-wide logger.  The pointer to the logger should not be
      cached or used at any time other than immediately after the
      invocation of this method. */
  REMOVABLE_INLINE LoggerI * logger()		{ return _logger; }

  /** Install a different logger for this plumber. This returns a pointer
      to the previous logger.  If non zero, it is the caller's
      responsibility to delete that logger if it's on the heap. */
  REMOVABLE_INLINE LoggerI * logger(LoggerI * newLogger);
  
  /** My logging level */
  LoggerI::Level loggingLevel;

private:
  /** The list of installed dataflows */
  boost::shared_ptr<std::map< string, DataflowPtr > > _dataflows;

  /** My local logger */
  LoggerI * _logger;
};

#endif
