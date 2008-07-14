#include <iostream>
#include <fstream>
#include <string>
#include "parseContext.h"

using namespace std;
using namespace compile::parse;

int main( int argc, const char* argv[] )
{
    istream* pstream;
        
    for (int count = 0; count < argc; count ++)
    { 
      printf("%s", argv[count]);
    } 

    pstream = new ifstream(argv[1]);
       
    Context *context = new Context(string("p2"), true);

    context->testParse(pstream);

}

