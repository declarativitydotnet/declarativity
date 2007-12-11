#include "val_str.h"
#include "oniStageProcessor.h"
#include <dlfcn.h>

typedef std::map< std::string, // .so name
		  void *,      // .so handle
		  std::less< std::string> > SoSet;

static SoSet * _sharedObjects;

void OniStageFactoryInit() {
  _sharedObjects = new SoSet();
}

Stage::Processor*
OniStageFactory(std::string stageProcessorName, Stage* theStageElement) {
  // Someone needs to implement a Windows version of this function.

  // What is the .so called?  It's liboni{dlname}.so.  If the stage
  // contains an '_', then dlname is the stuff before the '_'.
  // Otherwise, dlname is equal to the stage name, and there's only
  // one stage in the .so.

  // Lame - Getting cmake to #define something with "'s in its value
  // is difficult.
  int so = 0; int dll = 1; int dylib = 2;
  char * suffix;
  switch(LIB_SUFFIX) {
  case 0: suffix = "so"; break;
  case 1: suffix = "dll"; break;
  case 2: suffix = "dylib"; break;
  default: abort();
  }

  std::string::size_type pos = stageProcessorName.find_first_of("_");
  std::string dlname = ("liboni" + stageProcessorName.substr(0, pos) + "." + suffix);
  std::string funcname = ("_Oni" + stageProcessorName + "Func");

  void * dlhandle = 0;
  if(pos != std::string::npos) {
    SoSet::iterator i = _sharedObjects->find(dlname);
    if(i != _sharedObjects->end()) {
      dlhandle = (*i).second;
    }
  }
  if(!dlhandle) {
    dlhandle = dlopen(dlname.c_str(), RTLD_NOW);
    if(!dlhandle) {
      char * dlerr = dlerror();
      TELL_ERROR << (stageProcessorName + " Couldn't load stage.  dlopen: " + dlerr + "\n");
      throw StageRegistry::StageNotFound(stageProcessorName + " Couldn't load stage. dlopen: " + dlerr + "\n");
    }
  }
  if(pos != std::string::npos) {
    // remember that we dlopened this already...
    _sharedObjects->insert(std::make_pair(dlname,dlhandle));
  }

  void * stagefunc = dlsym(dlhandle, funcname.c_str());

  if(!stagefunc) {
    TELL_ERROR << (stageProcessorName + " Couldn't load stage.  dlopen successfully opened "
		   + dlname + ", but dlsym did not find "
		   + funcname + ".\n");
    throw StageRegistry::StageNotFound(stageProcessorName + " Couldn't load stage.  dlopen successfully opened "
				       + dlname + ", but dlsym did not find "
				       + funcname + ".\n");
  }

  Stage::Processor* processor = new OniStageProcessor(theStageElement, stageProcessorName, stagefunc);

  return processor;
}

OniStageProcessor::OniStageProcessor(Stage *myStage, std::string stageName, void * dlfunc) :
  Stage::Processor(myStage),
  _stageName(stageName),
  _dlfunc((int(*)(void*,void*))dlfunc),
  _ready(false) {}

void OniStageProcessor::newInput(TuplePtr inputTuple) {
  assert(!_ready);
  TuplePtr t(new Tuple());
  t->append(Val_Str::mk(_stageName));
  t->append((*inputTuple)[1]);
  int err = _dlfunc((void*)boost::get_pointer(inputTuple),
		    (void*)boost::get_pointer(t));
  if(err) {
    TELL_ERROR << _stageName << " C function returned error code " << err << "\n";
    _ready=false;
  } else {
    t->freeze();
    _ready=true;
    _out = t;
  }
}
std::pair< TuplePtr, Stage::Status > OniStageProcessor::newOutput() {
  if(!_ready) {
    return std::make_pair(TuplePtr(),Stage::DONE);
  } else {
    _ready = false;
    return std::make_pair(_out,Stage::MORE);
  }
}
