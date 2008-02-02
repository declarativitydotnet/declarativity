/* 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#include "boost/test/unit_test.hpp"

#include <sstream>

#include "fdbuf.h"
#include "sfslite.h"
#include "testSecure.h"

using namespace compile::secure;

class testSecure
{
private:


public:
   testSecure()
   {
   }
   
   
public:


// ==================
// key serialization and deserialization tests
// ==================

void testRSAKeySerialize() 
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "RSA Key serialization test.";
      testID = ID.str();
   }
   //test private serialization and deserialization
   for(int i = 0; i < 10; i++){
     std::ostringstream message1;
     message1<<testID;
     std::ostringstream message2;
     message2<<testID;

     bool res1, res2;
     Sfslite::testSerialization(res1, message1, res2, message2);
     BOOST_CHECK_MESSAGE(res1, message1.str().c_str());

     BOOST_CHECK_MESSAGE(res2, message2.str().c_str());

   }
}

void testRSAKeyFileSerialize() 
{
  std::string privFile = "key.priv";
  std::string pubFile = "key.pub";

   std::string testID;
   {
      std::ostringstream ID;
      ID << "RSA Key serialization test.";
      testID = ID.str();
   }
   //test private serialization and deserialization
   for(int i = 0; i < 10; i++){
     std::ostringstream message1;
     message1<<testID;
     std::ostringstream message2;
     message2<<testID;

     bool res1, res2;
     Sfslite::testFileSerialization(res1, message1, res2, message2, privFile, pubFile);
     BOOST_CHECK_MESSAGE(res1, message1.str().c_str());

     BOOST_CHECK_MESSAGE(res2, message2.str().c_str());

   }
}


// ==================
// RSA encryption tests
// ==================

void testRSAEncryptionDecryption() 
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "RSA Key encryption/decryption test.";
      testID = ID.str();
   }
   //test private serialization and deserialization
   for(int i = 0; i < 10; i++){
     bool correct = Sfslite::testRSA();
     
     std::ostringstream message1;
     message1<< testID
	     << "The msg was incorrectly encrypted-decrypted ";
     
     BOOST_CHECK_MESSAGE(correct, message1.str().c_str());

   }
}

// ==================
// RSA encryption tests
// ==================

void testAESEncryptionDecryption() 
{
   std::string testID;
   {
      std::ostringstream ID;
      ID << "AES encryption/decryption test.";
      testID = ID.str();
   }
   //test private serialization and deserialization
   for(int i = 0; i < 10; i++){
     bool correct = Sfslite::testAES();
     
     std::ostringstream message1;
     message1<< testID
	     << "The msg was incorrectly encrypted-decrypted ";
     
     BOOST_CHECK_MESSAGE(correct, message1.str().c_str());

   }
}

};

testSecure_testSuite::testSecure_testSuite() 
  : boost::unit_test_framework::test_suite("testSecure")
{
  boost::shared_ptr<testSecure> instance(new testSecure());
  add(BOOST_CLASS_TEST_CASE(&testSecure::testRSAKeySerialize, instance));
  add(BOOST_CLASS_TEST_CASE(&testSecure::testRSAKeyFileSerialize, instance));
  add(BOOST_CLASS_TEST_CASE(&testSecure::testRSAEncryptionDecryption, instance));
  add(BOOST_CLASS_TEST_CASE(&testSecure::testAESEncryptionDecryption, instance));
}
