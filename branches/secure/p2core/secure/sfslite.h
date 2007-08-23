#include "fdbuf.h"

namespace compile {

  namespace secure {
    // hash size in bytes
    const int hashSize = 20;

    FdbufPtr secureSignAES(FdbufPtr msg, FdbufPtr key);
      
    FdbufPtr secureSignRSA(FdbufPtr msg, FdbufPtr key);
      
    bool secureVerifyAES(FdbufPtr msg, FdbufPtr key, FdbufPtr proof);
    
    bool secureVerifyRSA(FdbufPtr msg, FdbufPtr key, FdbufPtr proof);

  };
};
