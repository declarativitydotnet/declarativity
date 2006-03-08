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
#include <element.h>
#include <elementSpec.h>
#include <loggerI.h>

/** A handy dandy type for plumber references */
class Plumber;
typedef boost::shared_ptr< Plumber > PlumberPtr;


class Plumber {
public:
  
  /** An auxilliary structure to help pass around element connection
      specifications */
  struct Hookup {
    /** The element from which this hookup originates */
    ElementSpecPtr fromElement;
    
    /** The port number at the fromElement */
    int fromPortNumber;

    /**  The element to which this hookup goes */
    ElementSpecPtr toElement;

    /** The port number at the toElement */
    int toPortNumber;

    Hookup(ElementSpecPtr fe, int fp,
           ElementSpecPtr te, int tp)
      : fromElement(fe), fromPortNumber(fp),
        toElement(te), toPortNumber(tp) {};
  };
  typedef boost::shared_ptr< Hookup > HookupPtr;

  struct Configuration {
    /** The elements */
    std::vector< ElementSpecPtr > elements;

    /** The hookups */
    std::vector< HookupPtr > hookups;

    ElementSpecPtr addElement(ElementPtr e) {
      ElementSpecPtr r(new ElementSpec(e));
      elements.push_back(r);
      return r;
    }
    void hookUp(ElementSpecPtr src, int src_port,
		ElementSpecPtr dst, int dst_port ) {
      HookupPtr p(new Hookup(src, src_port, dst, dst_port));
      hookups.push_back(p);
    }

    Configuration() {};
    Configuration(boost::shared_ptr< std::vector< ElementSpecPtr > > e,
                  boost::shared_ptr< std::vector< HookupPtr > > h)
      : elements(*e), hookups(*h) {};

  };
  typedef boost::shared_ptr< Configuration > ConfigurationPtr;

  /** Create a new plumber given a configuration of constructed but not
      necessarily configured elements. */
  Plumber(ConfigurationPtr configuration,
         LoggerI::Level loggingLevel = LoggerI::INFO);

  ~Plumber();

  // INITIALIZATION
  
  /** Initialize the engine from the configuration */
  int initialize(PlumberPtr);

  /** Start the plumber */
  void activate();





  static void static_initialize();
  static void static_cleanup();

  /** Plumber state */
  enum { PLUMBER_NEW,
         PLUMBER_PRECONFIGURE,
         PLUMBER_PREINITIALIZE,
         PLUMBER_LIVE,
         PLUMBER_DEAD };

  bool initialized() const			{ return _state == PLUMBER_LIVE; }

  // ELEMENTS
  int nelements() const				{ return _elements->size(); }
  const boost::shared_ptr< std::vector< ElementPtr > > elements() const { return _elements; }
  
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
  
  boost::shared_ptr< std::vector< ElementPtr > > _elements;
  
  /** The plumber state */
  int _state;

  /** The configuration spec of the plumber */
  ConfigurationPtr _configuration;

  /** Are the configuration hookups refering to existing elements and
      non-negative ports? */
  int check_hookup_elements();

  /** Are the port numbers within the attached element's range? */
  int check_hookup_range();

  /** Is personality semantics correctly applied to hookups?  This only
      checks that the end-points of a hookup are consistent.  It does
      not follow flow codes through elements. */
  int check_push_and_pull();

  /** Are any ports multiply connected?  Are all ports attached to
      something?  We require all ports to be attached to something
      (exactly one something), even pull outputs and push inputs. */
  int check_hookup_completeness();

  /** Perform the actual hooking up of real elements from the specs. No
      error checking at this point.  */
  void set_connections();

  /** Convenience function for adding a created (but not initialized)
      element into the plumber. */
  void add_element(PlumberPtr, ElementPtr);

  /** My local logger */
  LoggerI * _logger;
};

#endif
