// -*- c-basic-offset: 2; related-file-name: "oni.C" -*-
/*
 * @(#)$$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Overlog Native Interface
 *
 */

#ifndef _ONI_H_
#define _ONI_H_

// Is this a C compiler?
#if defined(__STDC__) || defined(__cplusplus) || defined(_MSC_EXTENSIONS)
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

  typedef void val_t;
  typedef void tup_t;

  int64_t int_Val(val_t* v);
  const char *  str_Val(val_t* v);
  void *  opaque_Val(val_t* v, int64_t *size);

  val_t* alloc_Val_int(int64_t);
  void free_Val_int(val_t*);
  val_t* alloc_Val_str(const char*);
  void free_Val_str(val_t*);
  val_t* alloc_Val_opaque(void * v, int64_t size);
  void free_Val_opaque(val_t*);
  val_t* Val_Tup(tup_t *, int64_t i);
  int64_t colCount_Tup(tup_t *);
  void Tup_Push(tup_t *, val_t*);
  void Tup_Push_int(tup_t *, int);
  void Tup_Push_str(tup_t *, const char*);
  void Tup_Push_opaque(tup_t *, void*, int64_t size);
  void Tup_Push_Null(tup_t*);
  static const int ONI_INFO_NUM = 1;
  static const int ONI_WORDY_NUM = 2;
  static const int ONI_WARN_NUM = 3;
  static const int ONI_ERROR_NUM = 4;
  static const int ONI_OUTPUT_NUM = 5;

  void oni_log(int type, char * fmt,...);

#define oni_info(fmt,...) oni_log(ONI_INFO_NUM,fmt,__VA_ARGS__)
#define oni_wordy(fmt,...) oni_log(ONI_WORDY_NUM,fmt,__VA_ARGS__)
#define oni_warn(fmt,...) oni_log(ONI_WARN_NUM,fmt,__VA_ARGS__)
#define oni_error(fmt,...) oni_log(ONI_ERROR_NUM,fmt,__VA_ARGS__)
#define oni_output(fmt,...) oni_log(ONI_OUTPUT_NUM,fmt,__VA_ARGS__)


#ifdef __cplusplus
} // extern "C"
#endif

#define ONI_TUP_STAGE(X) 						\
  int _Oni##X##Func(tup_t * tup, tup_t *ret)				\

#define END_ONI_STAGE


#else
#ifdef __OVERLOG__

#define ONI_TUP_STAGE(X)                                                \
  stage(""X"",X##_in,X##_out).

#define END_ONI_STAGE

#else
#error oni.h cannot figure out if it is being compiled as C or overlog.
#endif // __OVERLOG__
#endif // __STDC__

#endif // _ONI_H_
