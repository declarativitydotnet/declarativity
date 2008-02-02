#include "stasis.h"
#include <stdlib.h>
ONI_TUP_STAGE(stasis_init) {
  Tinit();
  Tup_Push(ret,Val_Tup(tup,2)); // request
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_deinit) {
  Tdeinit();
  Tup_Push(ret,Val_Tup(tup,2)); // request
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_begin) {
  int xid = Tbegin();
  Tup_Push(ret,Val_Tup(tup,2)); // request
  Tup_Push_int(ret,xid);
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_commit) {
  int xid = int_Val(Val_Tup(tup,3));
  Tcommit(xid);
  Tup_Push(ret,Val_Tup(tup,2)); // request
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_abort) {
  int xid = int_Val(Val_Tup(tup,3));
  Tabort(xid);
  Tup_Push(ret,Val_Tup(tup,2)); // request
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_prepare) {
  int xid = int_Val(Val_Tup(tup,3));
  Tprepare(xid,NULLRID);
  Tup_Push(ret,Val_Tup(tup,2)); // request
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_create_hash) {
  int xid = int_Val(Val_Tup(tup,3));
  recordid rid = ThashCreate(xid, VARIABLE_LENGTH, VARIABLE_LENGTH);
  Tup_Push(ret,Val_Tup(tup,2)); // request
  Tup_Push_recordid(ret,rid);
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_insert_hash) {
  int xid = int_Val(Val_Tup(tup,3));
  recordid rid = recordid_Val(Val_Tup(tup,4));
  int cnt = int_Val(Val_Tup(tup,5));
  size_t key_len;
  byte* key = colsBytes_Tup(tup,6,cnt,&key_len);
  size_t val_len;
  byte* val = colsBytes_Tup(tup,6+cnt,colCount_Tup(tup)-(6+cnt),&val_len);
  // retval is 1 if key was already defined...
  int retval = ThashInsert(xid,rid,key,key_len,val,val_len);
  Tup_Push(ret,Val_Tup(tup,2)); // request
  Tup_Push_int(ret,retval);
  return 0;
} END_ONI_STAGE
ONI_TUP_STAGE(stasis_lookup_hash) {
  int xid = int_Val(Val_Tup(tup,3));
  recordid rid = recordid_Val(Val_Tup(tup,4));
  int cnt = int_Val(Val_Tup(tup,5));
  size_t key_len;
  byte* key = colsBytes_Tup(tup,6,cnt,&key_len);
  byte* val;
  size_t val_len = ThashLookup(xid,rid,key,key_len,&val);
  if(val_len != -1) {
    Tup_Push(ret,Val_Tup(tup,2)); // request
    Tup_Push_int(ret,cnt);
    Tup_Push_colsBytes(ret,key,key_len);
    Tup_Push_colsBytes(ret,val,val_len);
    free(val);
  } else {
    int i;
    Tup_Push(ret,Val_Tup(tup,2)); // request
    // 5 is the column in tup that should contain a count.  P2 wants
    // us to return a consistent number of columns, so we pad the
    // output with extra nulls.
    for(i = 5; i < colCount_Tup(tup) - 6; i++) {
      Tup_Push_Null(ret);
    }
  }
  return 0;
} END_ONI_STAGE
