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
    

    const uint32_t RSA = 0, AES = 1;
    const uint32_t RSAPriv = 2, RSAPub = 3, AESSecret = 4;
    
    inline void
    b16xor (b16 *d, const b16 &s)
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
      serializedPriv->push_uint32(RSAPriv);
      serializedPriv->push_uint32(sizeP);
      serializedPriv->push_bytes(pbuf, sizeP);
      serializedPriv->push_uint32(sizeQ);
      serializedPriv->push_bytes(qbuf, sizeQ);
      serializedPriv->push_uint64(priv->k);
      delete pbuf;
      delete qbuf;
      return serializedPriv;

    }

    esign_priv* deserializePriv(FdbufPtr privBuf)
    {
      //create a reader copy
      FdbufPtr serializedPriv(new Fdbuf(privBuf));
      assert(serializedPriv->pop_uint32()==RSAPriv);
      uint32_t sizeP = serializedPriv->pop_uint32();
      bigint p, q;
      mpz_set_raw (&p, serializedPriv->raw_inline(), sizeP);
      uint32_t sizeQ = serializedPriv->pop_uint32();
      mpz_set_raw (&q, serializedPriv->raw_inline(), sizeQ);
      uint64_t k = serializedPriv->pop_uint64();
      
      return new esign_priv(p, q, k);
    }

    FdbufPtr serializePub(esign_pub *pub)
    {
      FdbufPtr serializedPub(new Fdbuf(258));
      uint32_t sizeN = mpz_rawsize(&pub->n);
      
      char *nbuf = new char[sizeN];
      mpz_get_raw (nbuf, sizeN, &pub->n);
      serializedPub->push_uint32(RSAPub);
      serializedPub->push_uint32(sizeN);
      serializedPub->push_bytes(nbuf, sizeN);
      serializedPub->push_uint64(pub->k);
      delete nbuf;
      return serializedPub;
    }

    esign_pub* deserializePub(FdbufPtr pubBuf)
    {
      //create a reader copy
      FdbufPtr serializedPub(new Fdbuf(pubBuf));
      assert(serializedPub->pop_uint32()==RSAPub);
      uint32_t sizeN = serializedPub->pop_uint32();
      bigint n;
      mpz_set_raw (&n, serializedPub->raw_inline(), sizeN);
      uint64_t k = serializedPub->pop_uint64();
      
      return new esign_pub(n, k);    
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

      FdbufPtr secureSignAES(FdbufPtr msg, FdbufPtr key){
	aes ctx;
	
	ctx.setkey (key->raw_inline(), key->length());

	bigint z;
	str msgStr(msg->cstr(), msg->length());
	msg2bigint(&z, msgStr, hashSize*8);
	
	uint32_t size = mpz_rawsize(&z);
	//size rounded to 16 bytes
	uint32_t size16 = (size+6>>4)<<4;
	char* pbuf = new char[size16];
	char* cbuf = new char[size16];
	mpz_get_raw (pbuf, size, &z);
	// fill the remaining bytes with 0s
	memset(pbuf+size, 0, size16-size);

	cbcencrypt(&ctx, cbuf, pbuf, size16);
	FdbufPtr cipher(new Fdbuf(size16));
	cipher->push_bytes(cbuf, size16);
	
	delete []cbuf;
	delete []pbuf;
	
	return cipher;
      }
      
      FdbufPtr secureSignRSA(FdbufPtr msg, FdbufPtr key){
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
      
      bool secureVerifyAES(FdbufPtr msg, FdbufPtr key, FdbufPtr proof){	
	aes ctx;
	
	ctx.setkey (key->raw_inline(), key->length());

	bigint msgHash;
	str msgStr(msg->cstr(), msg->length());
	msg2bigint(&msgHash, msgStr, hashSize*8);

	uint32_t size = mpz_rawsize(&msgHash);
	//size rounded to 16 bytes
	uint32_t size16 = (size+6>>4)<<4;
	if(proof->length() != size16){
	  return false;
	}
	char* pbuf = new char[size16];
	char* decbuf = new char[size16];
	mpz_get_raw (pbuf, size, &msgHash);
	memset(pbuf+size, 0, size16-size);
	
	cbcdecrypt(&ctx, decbuf, proof->raw_inline(), size16);
	bool res = memcmp(decbuf, pbuf, size16);
	delete []decbuf;
	delete []pbuf;

	return res;
      }
      
      bool secureVerifyRSA(FdbufPtr msg, FdbufPtr key, FdbufPtr proofbuf){
	FdbufPtr proof(new Fdbuf(proofbuf));
	esign_pub* pub = deserializePub(key);

	bigint msgHash;
	mpz_set_raw (&msgHash, proof->raw_inline(), proof->length());

	str msgStr(msg->cstr(), msg->length());
	bool res = pub->verify(msgStr, msgHash);

	delete pub;
	return res;
      }


  }
}
