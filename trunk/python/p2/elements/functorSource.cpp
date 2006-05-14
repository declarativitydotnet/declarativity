#include <functorSource.h>
#include <boost/python.hpp>
using namespace boost::python;

class GeneratorWrap : public FunctorSource::Generator, 
                      public wrapper<FunctorSource::Generator>
{
public:
  virtual TuplePtr operator()() {
    return this->get_override("operator()")();
  };
};

void export_functorSource()
{
scope outer =
  class_<FunctorSource, bases<Element>, boost::shared_ptr<FunctorSource>, boost::noncopyable>
        ("FunctorSource", init<std::string, FunctorSource::Generator*>())
    .def("class_name", &FunctorSource::class_name)
    .def("processing", &FunctorSource::processing)
    .def("flow_code",  &FunctorSource::flow_code)
  ;

  class_<GeneratorWrap, GeneratorWrap*, boost::noncopyable>
        ("Generator", init<>())
    .def("operator()", &GeneratorWrap::operator())
  ;
}
