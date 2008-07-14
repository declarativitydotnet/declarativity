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

#include <element.h>
#include <elementSpec.h>
#include <loggerI.h>
#include <dot.h>
#include <table2.h>

#include <inlines.h>
#include <fstream>

/** A handy dandy type for plumber references */
class Plumber;
typedef boost::shared_ptr< Plumber > PlumberPtr;

class Plumber {
public:
  class Dataflow : public Element {
  public:
    Dataflow(string name = "dataflow", 
             unsigned ninputs = 0, unsigned noutputs = 0, 
             string p = string(), string fc = string()); 

    virtual ~Dataflow();

    const char*
    class_name() const {return "Dataflow";}

    const char*
    processing() const {return _processing;}

    const char*
    flow_code() const {return _flow_code;}

    void toDot(std::ostream*);

    ElementSpecPtr
    input(unsigned* port);

    bool
    checkInput(ElementSpecPtr element, unsigned port);

    ElementSpecPtr
    output(unsigned* port);
    
    bool
    checkOutput(ElementSpecPtr element, unsigned port);

    /** The string representation of each element that is part 
     *  of this dataflow */
    string toString() const;

    /** Validate this dataflow. These checks are local to
        this dataflow (they don't require info from other dataflows). */
    virtual int
    validate();

    /** Overriden Element class method. A call to this method will
      * finalize the dataflow by actually establishing the port
      * connections between elements. */
    int initialize();

    /** Add a new element to this dataflow by creating a
        new ElementSpecPtr that references the passed in element.  */
    virtual ElementSpecPtr
    addElement(ElementPtr);


    /** Add hookup to this dataflow */
    virtual void
    hookUp(ElementSpecPtr src, unsigned src_port,
           ElementSpecPtr dst, unsigned dst_port );

    /** Find an element in existing dataflow based on the element
        name. If no such element is found, return a null pointer. */
    ElementSpecPtr find(string name);

    /** Regular hookups */
    std::vector< ElementSpec::HookupPtr > hookups_;

    /** Hookups specific to the inputs of this dataflow */
    std::vector< ElementSpec::HookupPtr > inputs_;

    /** Hookups specific to the outputs of this dataflow */
    std::vector< ElementSpec::HookupPtr > outputs_;

    /**
     *  All elements are local to this dataflow regardless of operation.
     *  Elements in other dataflows under INSERT and REMOVE operations
     *  will be indicated in the hookups.  */
    std::vector< ElementSpecPtr > elements_;

    /** Garbage elements removed from the dataflow. We need to do this
        given that these elements may have outstanding callbacks and therefore
        can't be destroyed. A fix for this is in the pipe. */
    std::vector< ElementSpecPtr > garbage_elements_;

    /** Obtain the tuple injector for this dataflow. If the returned
        pointer is null, no element has been designated as a tuple
        injector.  */
    ElementPtr
    injector();


    /** Replace the tuple injector for this dataflow */
    void
    injector(ElementPtr tupleInjectorIPtr);




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
     *  garbage collected.  */
    virtual int check_hookup_completeness();

    /** Are any ports multiply connected?  Are all ports attached to
     * something?  We require all ports to be attached to something
     * (exactly one something), even pull outputs and push inputs. */
    virtual int eval_hookups();

    /** Perform the actual hooking up of real elements from the specs. No
        error checking at this point.  */
    virtual void set_connections();

  private:
    /** Static port connection */
/*
    int connect_input(unsigned i, Element *f, unsigned port);
    int connect_output(unsigned o, Element *f, unsigned port);
*/

    const char * _processing;
    const char * _flow_code;

    bool _validated;
    bool _initialized;


    /** My tuple injector */
    ElementPtr _injector;
  };

  typedef boost::shared_ptr< Dataflow > DataflowPtr;



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

  /** Create a new plumber given a configuration of constructed but not
      necessarily configured elements. */
  Plumber();

  /** Return a reference to a new dataflow object */
  static DataflowEditPtr
  edit(string name);


  /** Install the dataflow in this plumber. 
   *  Return: 0 on success, -1 on failure.  */
  static int install(DataflowPtr d);

  /** Output in .dot format a representation of this dataflow */
  static void toDot(string f);

  static int ndataflows() { return _dataflows.size(); }

  REMOVABLE_INLINE static DataflowPtr
  dataflow(string n);


  // LOGGING infrastructure
  
  /** The plumber-wide logger.  The pointer to the logger should not be
      cached or used at any time other than immediately after the
      invocation of this method. */
  REMOVABLE_INLINE LoggerI * logger() { return _logger; }

  /** Install a different logger for this plumber. This returns a pointer
      to the previous logger.  If non zero, it is the caller's
      responsibility to delete that logger if it's on the heap. */
  REMOVABLE_INLINE LoggerI * logger(LoggerI * newLogger);

  static CommonTable::ManagerPtr catalog() 
  { return _catalog; }
  
private:
  static CommonTable::ManagerPtr _catalog;

  /** The list of installed dataflows */
  static std::map< string, DataflowPtr > _dataflows;

  /** My local logger */
  LoggerI * _logger;
};

#endif
