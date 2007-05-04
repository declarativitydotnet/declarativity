#ifndef __VAL_TIME_H__
#define __VAL_TIME_H__

#include "value.h"
#include "Sketches.h"
#include <boost/shared_ptr.hpp>
#include <math.h>

typedef boost::shared_ptr<Sketches::CountMinFM> SketchPtr;

class Val_Sketch : public Value
{

public:
  const Value::TypeCode typeCode() const
  {
    return Value::SKETCH;
  };

  const char *typeName() const
  {
    return "sketch";
  };

  virtual string toConfString() const;
  virtual string toString() const;

  virtual unsigned int size() const;

  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  /* Construct a sketch with a given id that will produce the given error bound
    (on _frequently occurring_ items) with a given probability. */
  Val_Sketch(double errorBound, double errorProbability);
  virtual ~Val_Sketch()
  {
    delete map;
  };

  // Factory
  static ValuePtr mk(double errorBound, double errorProbability)
  {
    return ValuePtr(new Val_Sketch(errorBound, errorProbability));
  }

  // Casting methods are only relevant to extract the underlying sketch.
  static ValuePtr cast(ValuePtr v);


  const ValuePtr toMe(ValuePtr other) const
  {
    return
      mk(cast(other));
  }

  int compareTo(ValuePtr other);

  /* Get the frequency of a frequently occurring item (this won't work with
    infrequent occurrences due to the nature of the sketch). */
  Val_UInt64 getFrequency(const std::str &objectName);

  void merge(ValuePtr other);

private:
  SketchPtr sketchPtr;
};