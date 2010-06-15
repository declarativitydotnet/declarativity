#include <stdio.h>
#include <stdlib.h>
// dont count TCP/IP overhead for now since our byte estimation
// in the code does not count that

#define TCPIP

double size_of_grant(int f){
  double result = 0;
  result = 56 + 20.0 * (2*f + 1);
  return result;

}

double size_of_cert(int f){
  double result = size_of_grant(f) * (2.0*f + 1.0);
  return result;
}

double size_of_write1(int f, int size_of_req){
  double result =  (36.0 + size_of_req);
#ifdef TCPIP
  result +=  44;
#endif
  return result;
  
}

double size_of_write1ok(int f){
  double result = 0;
  result = 24 + size_of_grant(f);
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

double size_of_write2(int f){
  double result = 0.0;
  result = 4.0 + size_of_cert(f);
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

double size_of_write2ans(int f, int size_of_res){
  double result = 40 + size_of_res;
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

double size_of_hq(int f, double cont, int size_of_req, int size_of_resp){
  
  double result =  (size_of_write1(f, size_of_req) + size_of_write1ok(f) +
	  size_of_write2(f) + size_of_write2ans(f, size_of_resp));
  
  return result;
}

// <rid, seq, cid, oid, opnum, ophash, mac>
double size_of_hint(int f, int batchsize){
  double result = 16+8+20+20;
  #ifdef TCPIP
  result += 44;
  #endif
  result /= batchsize;
  return result;
}
// <rid, seq, cid, oid, opnum, ophash, mac>
double size_of_hint_ack(int f, int batchsize){
  return size_of_hint(f, batchsize);
}

double size_of_hq_p_uhg(int f, int size_of_req, int size_of_resp, int batchsize){
  double result = (2.0*f+1)*size_of_hint(f, batchsize) + 
    size_of_hq(f, 0.0, size_of_req, size_of_resp);
  result += (2.0*f + 1) * size_of_hint_ack(f, batchsize);
  return result;
}

double size_of_hq_p_other(int f, int size_of_req, int size_of_resp, int batchsize){
  double result = size_of_hq(f, 0.0, size_of_req, size_of_resp) + size_of_hint(f, batchsize)
    + size_of_hint_ack(f, batchsize);
  return result;
}

// <Pre-prepare, rid, seq, v, <hash of requests>, mac>
double size_of_pre_prepare(int f, int batchsize){
  double result = 0;
  result = 4.0 * 2 + 8 + 20.0 * batchsize + 20;
  #ifdef TCPIP
  result += 44;
  #endif
  //printf("Preprepare = %f\n", result);
  return result/batchsize;
}

// <Prepare, rid, seq, v, hashop, mac>
// Protocol says that it should also contain the hashes
double size_of_prepare(int f, int batchsize){
  double result = 0;
  result = 4.0 * 2 + 8 + 20*batchsize + 20;
  #ifdef TCPIP
  result += 44;
  #endif
  //printf("Prepare = %f\n", result);
  return result/batchsize;
}

// <Commit, rid, seq, v, mac>
double size_of_commit(int f){
  double result = 0;
  result = 4.0 * 2 + 8 + 20;
  #ifdef TCPIP
  result += 44;
  #endif
  //printf("Commit = %f\n", result);
  return result;
}

// <Request, cid, t, O, mac>
double size_of_pbft_req(int size_of_req){
  double result = 4 + 8 + size_of_req + 20;
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

// <Reply, v, t, c, i, r, mac>
double size_of_pbft_resp(int size_of_resp){
  double result = 8 + 8 + 4 + 8 + size_of_resp + 20;
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

double size_of_pbft_primary(int f, int size_of_req, int size_of_resp, int batchsize){
  double result = 0;
  result += size_of_pre_prepare(f, batchsize) * (2*f+1) + size_of_prepare(f, batchsize) 
    * (2.0 * f + 1) + size_of_commit(f) * 2 * (2*f+1);
  result += size_of_pbft_req(size_of_req) + size_of_pbft_resp(size_of_resp);
  return result;
}

double size_of_pbft_non_primary(int f, int size_of_req, int size_of_resp, int batchsize){
  double result = 0;
  result += size_of_pre_prepare(f, batchsize) + size_of_prepare(f, batchsize) * (4*f + 1)
    + size_of_commit(f) * 2 * (2*f + 1);
  result += size_of_pbft_req(size_of_req) + size_of_pbft_resp(size_of_resp);
  return result;
}

double size_of_pbft(int f, int size_of_req, int size_of_resp, int batchsize){
  double result = size_of_pre_prepare(f, batchsize) + size_of_prepare(f, batchsize)
    + size_of_commit(f) + size_of_pbft_req(size_of_req) + size_of_pbft_resp(size_of_resp);
  //printf("Size_of_pbft = %f\n", result);
  return result;
}

int num_msgs_pbft(int f){
  return (8*f + 2);
}

int num_msgs_hq(int f){
  return 4;
}

int num_msgs_hq_s_uhg(int f){
  return (4 + 2*f+ 2*f);
}

int num_msgs_hq_s_non_uhg(int f){
  return 6;
}

// <rid, seq, <cid, oid, opnum, ophash>, mac>
double size_of_batch_hint(int f, int batchsize){
  double result = 4 + 4;
  result += batchsize*(4+4+8+20);
  result += 20;

  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

// <rid, <cid, oid, opnum, ts>, vs, hash, auth>
double size_of_batch_grant(int f, int batchsize){
  double result = 4 + batchsize*(4+4+8+8);
  result += 8 + 20 + (2*f+1)*20;
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

double size_of_batch_cert(int f, int batchsize){
  return (2*f+1)*size_of_batch_grant(f, batchsize);
}


// <rid, BatchGrant>
double size_of_batch_write1ok(int f, int batchsize){
  double result = 4 + size_of_batch_grant(f, batchsize);
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

double size_of_batch_write2(int f, int batchsize){
  double result = 4 + size_of_batch_cert(f, batchsize);
  #ifdef TCPIP
  result += 44;
  #endif
  return result;
}

// UHG receives k requests, sends k responses: so 2k at least
// it sends one batchhint, gets one batch hint ack, sends one batchwrite1-ok
// and gets 1 batchwrite2
double num_msgs_hq_s_uhg_batching(int f, int batchsize){
  return (2*batchsize + 2*(2*f+1) + 1 + 1)/(batchsize * 1.0);
}

double num_msgs_hq_s_non_uhg_batching(int f, int batchsize){
  return (2*batchsize + 2 + 1 + 1)/(batchsize * 1.0);
}


double size_of_hq_s_uhg_batching(int f, int batchsize, int size_of_req, 
				 int size_of_resp){
  double result = 0;
  result = batchsize*(size_of_write1(f, size_of_req)) + 
    batchsize*(size_of_write2ans(f, size_of_resp)) + size_of_batch_write1ok(f, batchsize)  + size_of_batch_write2(f, batchsize)  + 
    + 2*(2*f+1)*(size_of_batch_hint(f, batchsize));
  return result/batchsize;
}

double size_of_hq_s_non_uhg_batching(int f, int batchsize, int size_of_req, 
				     int size_of_resp){
  double result;
  result = batchsize*(size_of_write1(f, size_of_req) ) + 
    batchsize*(size_of_write2ans(f, size_of_resp)) + size_of_batch_write1ok(f, batchsize) + size_of_batch_write2(f, batchsize) + 
    + 2*(size_of_batch_hint(f, batchsize));
  return result/batchsize;
}

int main(int argc, char ** argv){

  int f_max = 10;
  int f = 1;
  int reqsize = 20;
  double KILOBYTE = 1000.0;
  int batchsize = 2;
  if(argc != 3){
    printf("Need reqsize batchsize\n");
    exit(0);
  }
  reqsize = atoi(argv[1]);
  batchsize = atoi(argv[2]);

  /**
  printf("#F\t1\t2\t5\t10\n");
  for(; f <= f_max; f++){
    printf("%d\t%d\t%.1f\t%.1f\t%.1f\n", f, num_msgs_hq_s_uhg(f),
	   num_msgs_hq_s_uhg_batching(f, 2), 
	   num_msgs_hq_s_uhg_batching(f, 5), 
	   num_msgs_hq_s_uhg_batching(f, 10));
  }
  f = 1;
  */
  printf("#F\tHQ\tHQ++-1\tHQ++-2\tHQ++-5\tHQ++10\tPBFT-1\tPBFT-2\tPBFT-5\tPBFT-10\n");
  for(; f <= f_max; f++){
    printf("%d\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n", f, 
	   size_of_hq(f, 0.0, reqsize, reqsize)/KILOBYTE,
	   size_of_hq_p_uhg(f, reqsize, reqsize, batchsize)/KILOBYTE,
	   size_of_hq_s_uhg_batching(f, 2, reqsize, reqsize)/KILOBYTE,
	   size_of_hq_s_uhg_batching(f, 5, reqsize, reqsize)/KILOBYTE,
	   size_of_hq_s_uhg_batching(f, 10, reqsize, reqsize)/KILOBYTE,
	   size_of_pbft_primary(f, reqsize, reqsize, 1)/KILOBYTE,
	   size_of_pbft_primary(f, reqsize, reqsize, 2)/KILOBYTE,
	   size_of_pbft_primary(f, reqsize, reqsize, 5)/KILOBYTE,
	   size_of_pbft_primary(f, reqsize, reqsize, 10)/KILOBYTE);
    

  }

  /************* 
   ************ Uncomment this for byte sizes*/
  printf("#F\tHQ\tHQ++(UHG)\tHQ++(Non-UHG)\tPBFT(Prim)\tPBFT(Non-prim)\n");

  for(; f <= f_max; f++){
    printf("%d\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n", f, size_of_hq(f, 0.0, reqsize, reqsize)/KILOBYTE, 
	   size_of_hq_p_uhg(f, reqsize, reqsize, batchsize)/KILOBYTE,
	   size_of_hq_p_other(f, reqsize, reqsize, batchsize)/KILOBYTE,
	   size_of_pbft_primary(f, reqsize, reqsize, batchsize)/KILOBYTE, 
	   size_of_pbft_non_primary(f, reqsize, reqsize, batchsize)/KILOBYTE);
  }
  

  printf("#F\tHQ\tHQ++(UHG)\tHQ++(Non-UHG)\tPBFT\n");
  for(; f <= f_max; f++){
    printf("%d\t%d\t%d\t%d\t%d\n", f, num_msgs_hq(f), num_msgs_hq_s_uhg(f),
	   num_msgs_hq_s_non_uhg(f), num_msgs_pbft(f));
  }


  return 0;
}
