// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */


static std::map<void*, string> variables;

////////////////////////////////////////////////////////////
// Function to output into a dataflow graph specification
////////////////////////////////////////////////////////////

static string conf_comment(string comment)
{
  ostringstream oss;
  oss << "\t# " << comment << std::endl;
  return oss.str();
}

static string conf_valueVec(std::vector<ValuePtr>& values)
{
  ostringstream oss;
  oss << "{";
  for (std::vector<ValuePtr>::iterator iter = values.begin(); 
       iter != values.end(); iter++) {
    oss << (*iter)->toConfString();
    if (iter + 1 != values.end())
      oss << ", ";
  }
  if (values.size() == 0) oss << "value";
  oss << "}";
  return oss.str(); 
}

static string conf_StrVec(std::set<string> values)
{
  ostringstream oss;
  oss << "{";
  for (std::set<string>::iterator iter = values.begin(); 
       iter != values.end(); iter++) {
    oss << "\"" << *iter << "\"" ;
    if (++iter  != values.end())
      oss << ", ";
    --iter;
  }
  if (values.size() == 0) oss << "str";
  oss << "}";
  return oss.str(); 
}

static string conf_UIntVec(std::vector<unsigned>& values)
{
  ostringstream oss;
  oss << "{";
  for (std::vector<unsigned>::iterator iter = values.begin(); 
       iter != values.end(); iter++) {
    oss << *iter;
    if (iter + 1  != values.end())
      oss << ", ";
  }
  if (values.size() == 0) oss << "int";
  oss << "}";
  return oss.str(); 
}

template <typename T>
static string conf_var(T *key) {
  std::map<void*, string>::iterator iter = variables.find((void*)key);
  if (iter == variables.end())
    return "unknown";
  return iter->second;
}

template <typename T>
static string conf_assign(T *e, string obj)
{
  ostringstream oss;
  ostringstream var;
  string v;
  var << "variable_" << variables.size();

  // std::cout << "CONF ASSIGN: " << var.str() << " = " << obj << std::endl;
  std::map<void*, string>::iterator iter = variables.find((void*)e);
  if (iter == variables.end()) {
    variables.insert(std::make_pair((void*)e, var.str()));  
    v = var.str();
  } else v = iter->second;

  oss << "\tlet " << v << " = " << obj << ";" << std::endl;
  return oss.str();
}

template <typename T>
static string conf_call(T *obj, string fn, bool element=true) {
  ostringstream oss;
  string obj_var = conf_var((void*) obj);
  
  oss << "\tcall " << obj_var;
  if (element)
    oss << ".element()." << fn;
  else
    oss << "." << fn;
  return oss.str();
}

template <typename P1, typename P2>
static string conf_hookup(string toVar, P1 toPort, string fromVar, P2 fromPort)
{
  ostringstream oss;
  
  oss << "\t" << toVar << "[" << toPort << "] -> [" << fromPort << "]" << fromVar << ";" 
      << std::endl; 
  return oss.str();
}

static string conf_function(string fn)
{
  ostringstream oss;
  oss << fn << "()";
  return oss.str(); 
}

static bool checkString(string check) {
  if (check.compare(0, 9, "variable_") == 0) {
    return false;	// P2DL variable
  }
  else if (check.compare(0, 1, string("{")) == 0) {
    return false;	// Vector of values, int, or strings
  }
  else if (check.compare(0, 1, string("<")) == 0) {
    return false;	// A tuple of some sort.
 }
  return true;		// This is a string
}

static string switchQuotes(string s) {
  for (string::size_type loc = s.find("\"", 0);
       loc != string::npos; loc = s.find("\"", loc+2))
    s.replace(loc, 1, "\\\"");
  return s;
}

template <typename A>
static string conf_function(string fn, A arg0) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg0;

  string s = conf_function(fn);
  s.erase(s.end()-1);
  if ((typeid(arg0) == typeid(string) || 
       typeid(arg0) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << "\"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << arg0 << ")";
  return oss.str();
}
template <typename A, typename B>
static string conf_function(string fn, A arg0, B arg1) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg1;

  string s = conf_function(fn, arg0);
  s.erase(s.end()-1);
  if ((typeid(arg1) == typeid(string) || 
       typeid(arg1) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg1 << ")";
  return oss.str();
}
template <typename A, typename B, typename C>
static string conf_function(string fn, A arg0, B arg1, C arg2) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg2;

  TELL_INFO << "CONF FUNCTION ARG2: " << arg2 << std::endl;
  string s = conf_function(fn, arg0, arg1);
  s.erase(s.end()-1);
  if ((typeid(arg2) == typeid(string) || 
       typeid(arg2) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg2 << ")";
  return oss.str();
}
template <typename A, typename B, typename C, typename D>
static string conf_function(string fn, A arg0, B arg1, C arg2, D arg3) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg3;

  string s = conf_function(fn, arg0, arg1, arg2);
  s.erase(s.end()-1);
  if ((typeid(arg3) == typeid(string) || 
       typeid(arg3) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg3 << ")";
  return oss.str();
}

template <typename A, typename B, typename C, typename D, typename E>
static string conf_function(string fn, A arg0, B arg1, C arg2, D arg3, E arg4) 
{
  ostringstream oss;
  ostringstream arg_ss;
  arg_ss << arg4;

  string s = conf_function(fn, arg0, arg1, arg2, arg3);
  s.erase(s.end()-1);
  if ((typeid(arg4) == typeid(string) || 
       typeid(arg4) == typeid(const char*)) && 
      checkString(arg_ss.str()))
    oss << s << ", \"" << switchQuotes(arg_ss.str()) << "\"" << ")";
  else
    oss << s << ", " << arg4 << ")";
  return oss.str();
}
