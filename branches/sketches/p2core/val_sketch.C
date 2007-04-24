Val_Sketch::Val_Sketch(int id, const std::map<std::string, uint64_t> &map,
                       double errorBound, double errorProbability)
{
  size_t width = ceil(M_E / epsilon);
  size_t depth = ceil(log(1.0 / delta));

  SketchPtr s(new Sketches::CountMinFM(id, map, width, depth, 32, 64,
                                       Sketches::SHA1));
  sketchPtr = s;
}

string Val_Sketch::toConfString()
{
  ostringstream sb;
  sb << "Sketch " << sketch;
  return;
}

string Val_Sketch::toString()
{
  return this->toConfString();
}

unsigned int Val_Sketch::size()
{
  return this->map->size();
}

void Val_Sketch::xdr_marshal_subtype(XDR *x)
{
  // TODO: IMPLEMENT ME!
}

static ValuePtr Val_Sketch::xdr_unmarshal(XDR *x)
{
  // TODO: IMPLEMENT ME!
}


static SketchPtr Val_Sketch::cast(ValuePtr v)
{
  switch (v->typeCode)
  {
  case Value::SKETCH:
    {
      return(static_cast<Val_Matrix *>(v.get()))->sketch;
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

void Val_Sketch::getFrequency(const std::string &objectName)
{
  return this->sketch.getFrequency(objectName);
}

int Val_Sketch::compareTo(ValuePtr other)
{}