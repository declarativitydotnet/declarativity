#ifndef __VAL_SKETCH_H__
#define __VAL_SKETCH_H__

#include "value.h"
#include "val_int64.h"
#include <Sketches.h>
#include <boost/shared_ptr.hpp>
#include <math.h>

typedef boost::shared_ptr<Sketches::CountMinFM> SketchPtr;

class Val_Sketch : public Value
{

public:
  Val_Sketch(SketchPtr sp) : sketchPtr(sp) {};
  Val_Sketch(int id, double errorBound, double errorProbability);
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
  virtual ~Val_Sketch() {};

  // Factory
  static ValuePtr mk(SketchPtr sp) {ValuePtr p(new Val_Sketch(sp)); return p; };
  static ValuePtr mk(int id, double errorBound, double errorProbability)
  {
    return ValuePtr(new Val_Sketch(id, errorBound, errorProbability));
  };

  // Casting methods are only relevant to extract the underlying sketch.
  static SketchPtr cast(ValuePtr v);


  const ValuePtr toMe(ValuePtr other) const
  {
    return mk(cast(other));
  }

  int compareTo(ValuePtr other) const;

  /* Get the frequency of a frequently occurring item (this won't work with
    infrequent occurrences due to the nature of the sketch). */
  Val_Int64 getFrequency(const std::string &objectName);

  void merge(ValuePtr other);

private:
  SketchPtr sketchPtr;
  std::map<std::string, uint64_t> map;
  size_t width;
  size_t depth;
};

#endif //__VAL_SKETCH_H_