#include <demux.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_demux()
{
  class_<Demux, bases<Element>, boost::shared_ptr<Demux>, boost::noncopyable>
        ("Demux", init<std::string, std::vector<ValuePtr>, optional<unsigned> >())
    .def("class_name", &Demux::class_name)
    .def("processing", &Demux::processing)
    .def("flow_code",  &Demux::flow_code)
  ;
}
