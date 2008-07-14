#ifndef __SFSLITE_H__
#define __SFSLITE_H__
#include "fdbuf.h"
#include <sstream>

namespace compile {

  namespace secure {
    // hash size in bytes


    class Sfslite{
    private:
    
    public:

    enum encryptionType{RSA = 0, AES = 1};
    enum encryptionKey{RSAPriv = 2, RSAPub = 3, AESSecret = 4};

    static const int hashSize;

    static FdbufPtr generatePrivKey(int len);

    static FdbufPtr generateAESKey(int len);

    static FdbufPtr getPubKey(FdbufPtr priv);

    static FdbufPtr secureSignAES(FdbufPtr msg, FdbufPtr key);
      
    static FdbufPtr secureSignRSA(FdbufPtr msg, FdbufPtr key);
      
    static bool secureVerifyAES(FdbufPtr msg, FdbufPtr key, FdbufPtr proof);
    
    static bool secureVerifyRSA(FdbufPtr msg, FdbufPtr key, FdbufPtr proof);

    static bool testAES();

    static bool testRSA();

    static void testSerialization(bool &res1, std::ostringstream &message1, bool &res2, std::ostringstream &message2);

    static void testFileSerialization(bool &res1, std::ostringstream &message1, bool &res2, std::ostringstream &message2, std::string privFile, std::string pubFile);

    static FdbufPtr readFromFile(std::string filename);
    
    static void writeToFile(std::string filename, FdbufPtr f);

    };
  };
};

#endif /*SFSLITE*/
