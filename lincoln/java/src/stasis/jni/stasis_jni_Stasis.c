#include "stasis_jni_Stasis.h"
#include <stasis/transactional.h>
#include <stasis/page.h>

JNIEXPORT void JNICALL Java_stasis_jni_Stasis_init
  (JNIEnv *e, jclass c) {
  Tinit();
}
JNIEXPORT void JNICALL Java_stasis_jni_Stasis_deinit
  (JNIEnv *e, jclass c) {
  Tdeinit();
}
JNIEXPORT jlong JNICALL Java_stasis_jni_Stasis_begin
  (JNIEnv *e, jclass c) {
  jlong xid = Tbegin();
  return xid;
}
JNIEXPORT void JNICALL Java_stasis_jni_Stasis_commit
  (JNIEnv *e, jclass c, jlong xid) {
  Tcommit((int)xid);
}
JNIEXPORT void JNICALL Java_stasis_jni_Stasis_abort
  (JNIEnv *e, jclass c, jlong xid) {
  Tabort((int)xid);
}
JNIEXPORT void JNICALL Java_stasis_jni_Stasis_prepare
  (JNIEnv *e, jclass c, jlong xid) {
  Tprepare((int)xid);
}
JNIEXPORT jlong JNICALL Java_stasis_jni_Stasis_record_1type_1read
  (JNIEnv *e, jclass c, jlong xid, jlong page, jlong slot) {
  recordid rid = { (pageid_t) page, (slotid_t)slot, -1 };
  Page * p = loadPage((int) xid, rid.page);
  writelock(p->rwlatch,0);
  int type =  stasis_record_type_read(xid, p, rid);
  unlock(p->rwlatch);
  releasePage(p);
  return (jlong)type;

}
/*JNIEXPORT jlongArray JNICALL Java_stasis_jni_Stasis_hash_1create
  (JNIEnv *, jclass, jlong);
JNIEXPORT void JNICALL Java_stasis_jni_Stasis_hash_1delete
  (JNIEnv *, jclass, jlong, jlong, jlong);
JNIEXPORT jlong JNICALL Java_stasis_jni_Stasis_hash_1cardinality
  (JNIEnv *, jclass, jlong, jlong, jlong);
JNIEXPORT jbyteArray JNICALL Java_stasis_jni_Stasis_hash_1insert
  (JNIEnv *, jclass, jlong, jlong, jlong, jbyteArray, jbyteArray);
JNIEXPORT jbyteArray JNICALL Java_stasis_jni_Stasis_hash_1remove
  (JNIEnv *, jclass, jlong, jlong, jlong, jbyteArray);
JNIEXPORT jbyteArray JNICALL Java_stasis_jni_Stasis_hash_1lookup
  (JNIEnv *, jclass, jlong, jlong, jlong, jbyteArray);
JNIEXPORT jbyteArray JNICALL Java_stasis_jni_Stasis_hash_1iterator
  (JNIEnv *, jclass, jlong, jlong, jlong);
JNIEXPORT void JNICALL Java_stasis_jni_Stasis_iterator_1close
  (JNIEnv *, jclass, jbyteArray);
JNIEXPORT jboolean JNICALL Java_stasis_jni_Stasis_iterator_1next
  (JNIEnv *, jclass, jbyteArray);
JNIEXPORT jbyteArray JNICALL Java_stasis_jni_Stasis_iterator_1key
  (JNIEnv *, jclass, jbyteArray);
JNIEXPORT jbyteArray JNICALL Java_stasis_jni_Stasis_iterator_1value
  (JNIEnv *, jclass, jbyteArray);
JNIEXPORT void JNICALL Java_stasis_jni_Stasis_iterator_1tuple_1done
  (JNIEnv *, jclass, jbyteArray);
*/
