#ifndef _ONISTAGEPROCESSOR_H_
#define _ONISTAGEPROCESSOR_H_

#include "stage.h"
#include "stageRegistry.h"

class OniStageProcessor : public Stage::Processor {
public:
  // Note that we're intentionally avoiding the boost stage registry
  // macro stuff.  This stage processor is a fallback for stages that
  // aren't compiled in, and needs to bypass static stage registrations.
  OniStageProcessor(Stage *myStage, std::string stageName, void * dlfunc);
  void newInput(TuplePtr inputTuple);
  std::pair< TuplePtr, Stage::Status > newOutput();
  std::string class_name() { return _stageName; }
private:
  std::string _stageName;
  int (*_dlfunc)(void *, void *);
  bool _ready;
  TuplePtr _out;
};

Stage::Processor* OniStageFactory(std::string stageProcessorName, Stage* theStageElement);
void OniStageFactoryInit();

#endif  // _ONISTAGEPROCESSOR_H_
