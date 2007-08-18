#include "val_sketch.h"

Val_Sketch::Val_Sketch(int id, double errorBound, double errorProbability)
{
  this->id = id;
  width = ceil(M_E / errorBound);
  depth = ceil(log(1.0 / errorProbability));
  frozen = false;
}

string Val_Sketch::toConfString() const
{
  ostringstream sb;
  sb << "Sketch " << sketchPtr.get()->toString();
  return sb.str();
}

string Val_Sketch::toString() const
{
  return this->toConfString();
}

unsigned int Val_Sketch::size() const
{
  return this->map.size();
}

void Val_Sketch::xdr_marshal_subtype(XDR *x)
{
  // TODO: Once boost marshalling is merged, fill me in.
}

ValuePtr Val_Sketch::xdr_unmarshal(XDR *x)
{
  // TODO: Once boost marshalling is merged, fill me in.
}


SketchPtr Val_Sketch::cast(ValuePtr v)
{
  switch (v->typeCode())
  {
  case Value::SKETCH:
    {
      return(static_cast<Val_Sketch *>(v.get()))->sketchPtr;
    }
  default:
  {
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::SKETCH,
                           "sketch");
  }
  }
}

Val_Int64 Val_Sketch::getFrequency(const std::string &objectName)
{
  return sketchPtr.get()->getFrequency(objectName);
}

void Val_Sketch::insert(std::string key, uint64_t value)
{
  assert(!frozen);
  
  map[key] += value;
}

void Val_Sketch::merge(ValuePtr other)
{
  assert(frozen);
  
  if(other->typeCode() == Value::SKETCH)
  {
    sketchPtr->merge(*(cast(other).get()));
  }
}

void Val_Sketch::freeze()
{
  frozen = true;
  
  Sketches::CountMinFM *sketch = new Sketches::CountMinFM(id, map, width, 
      depth, 32, 64, Sketches::HT_UNIVERSAL);
  
  sketchPtr.reset(sketch);
}

// Currently, all sketches are equal. This is mainly because comparing two
// sketches together doesn't make a whole lot of sense. --ACR
int Val_Sketch::compareTo(ValuePtr other) const
{
  if (other->typeCode() < Value::SKETCH)
  {
    return -1;
  }
  else if (other->typeCode() > Value::SKETCH)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}