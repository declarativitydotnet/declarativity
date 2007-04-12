
enum NetStack
{
  DEST   = 0,	// Destination address
  SRC    = 1,	// Source address
  RTT    = 2,	// Measured round trip time
  TS     = 3,	// Source timestamp
  RRATE  = 4,	// Receive rate
  LRATE  = 5,	// Loss rate
  CUMSEQ = 6,	// Cumulative sequence number
  SEQ    = 7,	// Packet sequence number
  STACK_SIZE
};
