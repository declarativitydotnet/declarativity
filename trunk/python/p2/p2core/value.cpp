#include <value.h>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

using namespace boost::python;

class ValueWrap : public Value, public wrapper<Value>
{
public:
  virtual unsigned int size() const {
    return this->get_override("size")();
  };
  virtual const TypeCode typeCode() const {
    return this->get_override("typeCode")();
  };
  virtual const char *typeName() const {
    return this->get_override("typeName")();
  };
  virtual std::string toString() const {
    return this->get_override("toString")();
  };
  virtual int compareTo(ValuePtr) const {
    return this->get_override("compareTo")();
  };
protected:
  virtual void xdr_marshal_subtype(XDR*) const {
    this->get_override("xdr_marshal_subtype")();
  };
};

void export_value()
{
  class_<ValueWrap, ValuePtr, boost::noncopyable>
        ("Value", no_init)
    .def("size",      &Value::size)
    .def("typeCode",  &Value::typeCode)
    .def("typeName",  &Value::typeName)
    .def("toString",  &Value::toString)
    .def("equals",    &Value::equals)
    .def("compareTo", &Value::compareTo)
  ; 

  class_<std::vector<ValuePtr>, boost::shared_ptr<std::vector<ValuePtr> > >("ValueVec")
    .def(vector_indexing_suite<std::vector<ValuePtr> >())
  ;
}
