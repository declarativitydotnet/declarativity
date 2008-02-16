/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: 
 */

#ifndef __PROGRAM_LOADER_H__
#define __PROGRAM_LOADER_H__

#include "element.h"
#include "elementRegistry.h"


class ProgramLoader : public Element { 
public:
  
  ProgramLoader(string name);

  ProgramLoader(TuplePtr args);

  ~ProgramLoader();

  const char *class_name() const { return "ProgramLoader";}
  const char *processing() const { return "/h"; }
  const char *flow_code() const	 { return "/-"; }

  void program(string, string, string, std::vector<std::string>*, bool compileOnly=false);
  void program(string, string);
  void dot(string name);

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  struct Program {
    Program(string n, string f, string s, std::vector<std::string>* d, string p="") 
      : name(n), file(f), stage(s), defs(d), prog(p) {};
    string name;
    string file;
    string stage;
    std::vector<std::string>* defs;
    string prog;
  };
  typedef boost::shared_ptr<Program> ProgramPtr;
  std::vector<ProgramPtr> programs;
  std::vector<ProgramPtr>::iterator programIter;

  int initialize();
  void loader();
  void programUpdate(TuplePtr program);
  
  bool compileOnly;
  string dotFile;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __PROGRAM_LOADER_ */
