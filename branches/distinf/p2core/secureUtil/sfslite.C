#include<cstdlib>
#include<iostream>
#include "sfslite.h"
#include "crypt.h"
#include "aes.h"
#include "esign.h"
#include "bigint.h"

namespace compile {

  namespace secure {

    struct b16 {
      enum { nc = 16, nl = nc / sizeof (long) };
      union {
	char c[nc];
	long l[nl];
      };
    };
    
    void b16xor (b16 *d, const b16 &s)
    {
      for (int i = 0; i < b16::nl; i++)
	d->l[i] ^= s.l[i];
    }
    inline void
    b16xor (b16 *d, const b16 &s1, const b16 &s2)
    {
      for (int i = 0; i < b16::nl; i++)
	d->l[i] = s1.l[i] ^ s2.l[i];
    }
    
    const int Sfslite::hashSize = 20;
    void cbcencrypt (aes *cp, void *_d, const void *_s, int len)
    {
      assert (!(len & 15));
      len >>= 4;
      const b16 *s = static_cast<const b16 *> (_s);
      b16 *d = static_cast<b16 *> (_d);
      
      if (len-- > 0) {
	cp->encipher_bytes (d->c, (s++)->c);
	while (len-- > 0) {
	  b16 tmp;
	  b16xor (&tmp, *d++, *s++);
	  cp->encipher_bytes (d->c, tmp.c);
	}
      }
    }

    void cbcdecrypt (aes *cp, void *_d, const void *_s, int len)
    {
      assert (!(len & 15));
      len >>= 4;
      const b16 *s = static_cast<const b16 *> (_s) + len;
      b16 *d = static_cast<b16 *> (_d) + len;
      
      if (len-- > 0) {
	--s;
	while (len-- > 0) {
	  cp->decipher_bytes ((--d)->c, s->c);
	  b16xor (d, *--s);
	}
	cp->decipher_bytes ((--d)->c, s->c);
      }
    }

    FdbufPtr serializePriv(esign_priv *priv)
    {
      FdbufPtr serializedPriv(new Fdbuf(512));
      uint32_t sizeP = mpz_rawsize(&priv->p);
      uint32_t sizeQ = mpz_rawsize(&priv->q);
      
      char *pbuf = new char[sizeP];
      char *qbuf = new char[sizeQ];
      mpz_get_raw (pbuf, sizeP, &priv->p);
      mpz_get_raw (qbuf, sizeQ, &priv->q);
      serializedPriv->push_uint32(Sfslite::RSAPriv);
      serializedPriv->push_uint32(sizeP);
      serializedPriv->push_uint32(sizeQ);
      serializedPriv->push_uint64(priv->k);
      serializedPriv->push_bytes(pbuf, sizeP);
      serializedPriv->push_bytes(qbuf, sizeQ);
      delete pbuf;
      delete qbuf;
      return serializedPriv;

    }

    esign_priv* deserializePriv(FdbufPtr privBuf)
    {
      //create a reader copy
      FdbufPtr serializedPriv(new Fdbuf(privBuf));
      assert(serializedPriv->pop_uint32()==Sfslite::RSAPriv);
      uint32_t sizeP = serializedPriv->pop_uint32();
      uint32_t sizeQ = serializedPriv->pop_uint32();
      uint64_t k = serializedPriv->pop_uint64();
      bigint p, q;
      mpz_set_raw (&p, serializedPriv->raw_inline(), sizeP);
      mpz_set_raw (&q, serializedPriv->raw_inline() + sizeP, sizeQ);
      return new esign_priv(p, q, k);
    }

    FdbufPtr serializePub(esign_pub *pub)
    {
      FdbufPtr serializedPub(new Fdbuf(258));
      uint32_t sizeN = mpz_rawsize(&pub->n);
      char *nbuf = new char[sizeN];
      mpz_get_raw (nbuf, sizeN, &pub->n);
      serializedPub->push_uint32(Sfslite::RSAPub);
      serializedPub->push_uint32(sizeN);
      serializedPub->push_uint64(pub->k);
      serializedPub->push_bytes(nbuf, sizeN);
      delete nbuf;
      return serializedPub;
    }

    esign_pub* deserializePub(FdbufPtr pubBuf)
    {
      //create a reader copy
      FdbufPtr serializedPub(new Fdbuf(pubBuf));
      assert(serializedPub->pop_uint32()==Sfslite::RSAPub);
      uint32_t sizeN = serializedPub->pop_uint32();
      uint64_t k = serializedPub->pop_uint64();
      bigint n;
      mpz_set_raw (&n, serializedPub->raw_inline(), sizeN);
      
      return new esign_pub(n, k);    
    }
    
    FdbufPtr Sfslite::generatePrivKey(int len){
      esign_priv priv = esign_keygen(len);
      return serializePriv(&priv);
    }

    FdbufPtr Sfslite::getPubKey(FdbufPtr priv){
      esign_priv* privKey = deserializePriv(priv);
      FdbufPtr pubKey = serializePub(privKey);
      delete privKey;
      return pubKey;
    }
    
    FdbufPtr Sfslite::generateAESKey(int len){
      assert(!(len & 15));
      assert((len & (len-1)) == 0);
      wmstr wmsg (len);
      rnd.getbytes (wmsg, len);
      FdbufPtr key(new Fdbuf(len));
      key->push_bytes(wmsg.cstr(), len);
      return key;
    }

    void msg2bigint (bigint *resp, const str &msg, int bits)
    {
      assert (bits);
      bits--;
      const size_t bytes = bits + 7 >> 3;
      zcbuf buf (bytes);
      sha1oracle ora (bytes, 1);
      ora.update (msg.cstr (), msg.len ());
      ora.final (reinterpret_cast<u_char *> (buf.base));
      buf[bytes-1] &= 0xff >> (-bits & 7);
      mpz_set_rawmag_le (resp, buf, bytes);
    }
    
    FdbufPtr Sfslite::secureSignAES(FdbufPtr msg, FdbufPtr key){
      aes ctx;
      
        //assert that key length is a multiple of 16 bytes
      int length = key->length();
      assert(!(length & 15));
      assert((length & (length-1)) == 0);
      // keylength must be a power of 2
      //find the place of first 1 bit in binary of length
      int pos = 0;
      int mask = 0;
      length = length>>1;
      while(length){
	mask = mask<<1 | 1;
	pos++;
	length = length>>1;
      }
      ctx.setkey (key->raw_inline(), key->length());
      bigint z;
      str msgStr(msg->cstr(), msg->length());
      msg2bigint(&z, msgStr, hashSize*8);
      
      uint32_t size = mpz_rawsize(&z);
      //size rounded to length bytes
      
      uint32_t sizeRounded = (size+mask>>pos)<<pos;
      char* pbuf = new char[sizeRounded];
      char* cbuf = new char[sizeRounded];
      mpz_get_raw (pbuf, size, &z);
      // fill the remaining bytes with 0s
      if(sizeRounded > size)
	memset(pbuf+size, 0, sizeRounded-size);
      
      cbcencrypt(&ctx, cbuf, pbuf, sizeRounded);
      FdbufPtr cipher(new Fdbuf(sizeRounded));
      cipher->push_bytes(cbuf, sizeRounded);
      
      delete []cbuf;
      delete []pbuf;
      
      return cipher;
    }
    
    FdbufPtr Sfslite::secureSignRSA(FdbufPtr msg, FdbufPtr key){
      esign_priv* priv = deserializePriv(key);
      
      bigint msgHash;
      str msgStr(msg->cstr(), msg->length());
      bigint m = priv->sign (msgStr);
      
      uint32_t size = mpz_rawsize(&m);
      FdbufPtr enc(new Fdbuf(size));
      char* ebuf = new char[size];
      mpz_get_raw (ebuf, size, &m);
      enc->push_bytes(ebuf, size);
      delete[] ebuf;
      delete priv;
      return enc;
    }
    
    bool Sfslite::secureVerifyAES(FdbufPtr msg, FdbufPtr key, FdbufPtr proof){	
      aes ctx;
        //assert that key length is a multiple of 16 bytes
      int length = key->length();
      assert(!(length & 15));
      assert((length & (length-1)) == 0);
      // keylength must be a power of 2
      //find the place of first 1 bit in binary of length
      int pos = 0;
      int mask = 0;
      length = length>>1;
      while(length){
	mask = mask<<1 | 1;
	pos++;
	length = length>>1;
      }
      
      ctx.setkey (key->raw_inline(), key->length());
      
      bigint msgHash;
      str msgStr(msg->cstr(), msg->length());
      msg2bigint(&msgHash, msgStr, hashSize*8);
      
      uint32_t size = mpz_rawsize(&msgHash);
      //size rounded to length bytes
      
      uint32_t sizeRounded = (size+mask>>pos)<<pos;
      if(proof->length() != sizeRounded){
	return false;
      }
      char* pbuf = new char[sizeRounded + 1];
      char* decbuf = new char[sizeRounded + 1];
      mpz_get_raw (pbuf, size, &msgHash);
      if(sizeRounded > size)
	memset(pbuf+size, 0, sizeRounded-size);
      
      cbcdecrypt(&ctx, decbuf, proof->raw_inline(), sizeRounded);
      int res = memcmp(decbuf, pbuf, sizeRounded);
      delete []decbuf;
      delete []pbuf;
      
      return res == 0;
    }
    
    bool Sfslite::secureVerifyRSA(FdbufPtr msg, FdbufPtr key, FdbufPtr proofbuf){
      FdbufPtr proof(new Fdbuf(proofbuf));
      esign_pub* pub = deserializePub(key);
      
      bigint msgHash;
      mpz_set_raw (&msgHash, proof->raw_inline(), proof->length());
      
      str msgStr(msg->cstr(), msg->length());
      bool res = pub->verify(msgStr, msgHash);
      
      delete pub;
      return res;
    }

    bool Sfslite::testAES(){
     FdbufPtr key = generateAESKey(32);
     size_t len = rnd.getword ()%256 + 256;
     wmstr wmsg (len);
     rnd.getbytes (wmsg, len);
     FdbufPtr msg(new Fdbuf(len));
     msg->push_bytes(wmsg.cstr(), len);
     
     FdbufPtr encrypted = Sfslite::secureSignAES(msg, key);
     bool correct = Sfslite::secureVerifyAES(msg, key, encrypted);
     return correct;
    }

    bool Sfslite::testRSA(){
     FdbufPtr privBuf = generatePrivKey(424 + rnd.getword () % 256);
     size_t len = rnd.getword ()%256 + 256;
     wmstr wmsg (len);
     rnd.getbytes (wmsg, len);
     FdbufPtr msg(new Fdbuf(len));
     msg->push_bytes(wmsg.cstr(), len);
     
     FdbufPtr encrypted = Sfslite::secureSignRSA(msg, privBuf);
     FdbufPtr pubBuf = getPubKey(privBuf);
     bool correct = Sfslite::secureVerifyRSA(msg, pubBuf, encrypted);
     return correct;
    }

    void Sfslite::testSerialization(bool &res1, std::ostringstream &message1, bool &res2, std::ostringstream &message2){
     esign_priv priv = esign_keygen (424 + rnd.getword () % 256);
     FdbufPtr serializedPriv = serializePriv(&priv);
     FdbufPtr serializedPub = serializePub(&priv);
     esign_priv *dpriv = deserializePriv(serializedPriv);
     esign_pub *dpub = deserializePub(serializedPub);
     
     message1<< "The original private key was: p: " <<  priv.p.cstr() 
	     << "q: "<< priv.q.cstr() << "k: "<< priv.k 
	     << " was reported incorrectly as " << dpriv->p.cstr() 
	     << "q: "<< dpriv->q.cstr() << "k: "<< dpriv->k;
     
     res1 = (dpriv->p == priv.p) && (dpriv->q == priv.q) && (dpriv->k == priv.k);

     message2<< "The original public key was: n: " <<  priv.n.cstr() 
	     << "k: "<< priv.k 
	     << " was reported incorrectly as " << dpub->n.cstr() 
	     << "k: "<< dpub->k;
     
     res2 = (dpub->n == priv.n) && (dpub->k == priv.k);     
    }

    void Sfslite::testFileSerialization(bool &res1, std::ostringstream &message1, bool &res2, std::ostringstream &message2, std::string privFile, std::string pubFile){
     esign_priv priv = esign_keygen (424 + rnd.getword () % 256);
     FdbufPtr serializedPriv = serializePriv(&priv);
     FdbufPtr serializedPub = serializePub(&priv);
     FdbufPtr serializedPrivCopy(new Fdbuf(serializedPriv));
     FdbufPtr serializedPubCopy(new Fdbuf(serializedPub));

     writeToFile(privFile, serializedPrivCopy);
     FdbufPtr serializedPrivRead = readFromFile(privFile);
     
     res1 = (serializedPrivRead->str() == serializedPriv->str());

     writeToFile(pubFile, serializedPubCopy);
     FdbufPtr serializedPubRead = readFromFile(pubFile);
     
     res2 = (serializedPubRead->str() == serializedPub->str());

     message1<< "The original priv fdbuf was: " <<  serializedPriv->cstr() 
	     << " was read incorrectly as " << serializedPrivRead->cstr() ;

     message2<< "The original pub fdbuf was: " <<  serializedPub->cstr() 
	     << " was read incorrectly as " << serializedPubRead->cstr() ;
     
    }


    FdbufPtr Sfslite::readFromFile(std::string filename){
      int file = open(filename.c_str(), O_RDONLY, 0);
	//      std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary);
      Fdbuf *f = new Fdbuf();
      FdbufPtr fdbufPtr(f);
      if (file != -1)
	{
	  while(f->read(file) > 0);
	  close(file);
	}
      else std::cout << "Unable to open file";
      return fdbufPtr;
    }
    
    void Sfslite::writeToFile(std::string filename, FdbufPtr f){
      int file = open(filename.c_str(), O_WRONLY|O_TRUNC|O_CREAT, 0);
      //      std::ofstream file(filename.c_str(), std::ios::out|std::ios::binary);
      if (file != -1)
	{
	  while(f->write(file) > 0);
	  close(file);
	}
      else std::cout << "Unable to open file";
    }
    
  }
}
