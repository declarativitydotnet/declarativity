#ifndef __VAL_SKETCH_H__
#define __VAL_SKETCH_H__

#include "value.h"
#include "val_str.h"
#include "val_int64.h"
#include <Sketches.h>
#include <boost/shared_ptr.hpp>
#include <math.h>

typedef boost::shared_ptr<Sketches::CountMinFM> SketchPtr;

class Val_Sketch : public Value
{

public:
  Val_Sketch(SketchPtr sp) : sketchPtr(sp) 
  {
    frozen = true;
  };
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

  //Get the frequency of a frequently occurring item (this won't work with
  // infrequent occurrences due to the nature of the sketch).
  Val_Int64 getFrequency(const std::string &objectName);

  // Two sketches can be "merged" together to create a new sketch that is the 
  // union of the two sketches. This union is duplicate-insensitive.
  void merge(ValuePtr other);
  
  // Insert a value into the map which will feed the sketch. If two different 
  // values for the same key are inserted into the map, the key will 
  // refer to the sum of the two values. Value must be positive.
  void insert(std::string key, uint64_t value);
  
  // "Freezes" the sketch. Actual initialization of the underlying CountMinFM 
  // object doesn't happen until freeze() is called. You should only freeze 
  // a sketch after a reasonably large number of objects has been inserted 
  // into it, otherwise the sketch won't be very accurate.
  void freeze();

private:
  SketchPtr sketchPtr;
  std::map<std::string, uint64_t> map;
  size_t width;
  size_t depth;
  int id;
  bool frozen;
};

#endif //__VAL_SKETCH_H_