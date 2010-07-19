require 'bloom'

def localhost
  'localhost'
end

class TwoPC
  attr_accessor :status
  attr_reader :xid, :logic

  def initialize(members, xid)
    # configure relevant tables, queues and constants
    # all are immutable objects (though collection contents can change)
    @logic = Bloom.new
    @group = logic.table(:group, ['locspec'])
    @responses = logic.table(:responses, ['sender', 'msg'])
    @mq = logic.queue(:mq)
    @responseq = logic.queue(:responseq, ['sender', 'msg'])
    @xid = xid

    # set up group membership
    # members(X) <= group(X)
    members.each {|m| @group << [m]}
    
    # register rules
    init_coord
    }
  end
  
  def init_coord
    @peers = logic.table(:peers, [coord, peer])
    @peer_cnt = logic.table(:peer_cnt, [coord], [cnt])
    @yes_cnt = logic.table(:yes_cnt, [coord,xid], [cnt])
    @transaction = logic.table(:transaction, [coord,xid], [state])
    @vote = logic.table(:vote,[coord,xid,peer], [vote])
    
    logic.stratum[0] = rules {
      # Count number of peers
      # peer_cnt(Coordinator, count<Peer>) :- peers(Coordinator, Peer); 
      peers.reduce(peer_cnt) {|memo,p| memo[[p.coord]] += 1; memo}

      # Count number of "yes" votes
      # yes_cnt(Coordinator, TxnId, count<Peer>) :- vote(Coordinator, TxnId, Peer, Vote), 
      # Vote == "yes"; 
      vote.reduce(yes_cnt) {|memo,v| memo[[coord,xid]] += 1; memo}
    }
    
    logic.stratum[1] = rules {
      # /* Prepare => Commit if unanimous */ 
      # transaction(Coordinator, TxnId, "commit") :- peer_cnt(Coordinator, NumPeers), yes_cnt(Coordinator, TxnId, NumYes), transaction(Coordinator, TxnId, State), NumPeers == NumYes, State == "prepare"; 
      r1 = logic.join(peer_cnt, yes_cnt, transaction)
      transaction <= r1.map{|p,y,t| [p.coord, y.xid, 'commit'] if p.coord == y.coord and p.coord == t.coord and y.xid == t.xid and p.cnt = y.cnt and t.state = 'prepare'}
      
      # /* Prepare => Abort if any "no" votes */ 
      # transaction(Coordinator, TxnId, "abort") :- vote(Coordinator, TxnId, _, Vote), transaction(Coordinator, TxnId, State), Vote == "no", State == "prepare"; 
      r2 = logic.join(vote, transaction)
      transaction <= r2.map{|v,t| [v.coord, v.xid, 'abort'] if v.coord = t.coord and v.xid = t.xid and v.vote == 'no' and t.state == 'prepare' }

      # /* All peers know transaction state */ 
      # transaction(@Peer, TxnId, State) :- peers(@Coordinator, Peer), transaction(@Coordinator, TxnId, State); 
      r3 = logic.join(peers, transaction) 
      transaction <= r3.map {|p,t| [p.peer, t.xid, t.state] if p.coord == t.coord
        
      # /* Declare a timer that fires once per second */ 
      # timer(ticker, 1000ms);       
    }
  end
  
  # the remainder needs rewriting.

  def mcast(msg)
    # mq(X, localhost, Xid, M) <= group(X), vars('msg', M), vars('xid', Xid)
    rules {
      @mq <= @group.map{|r| [r.locspec, localhost, @xid, msg]}
    }
  end

  # Subordinate Logic
  def client
    
    @mq.rcvr.map do |m|
      case m.msg
        when 'prepare' then @responseq <= [localhost, :prepared]
        when 'commit' then @responseq <= [localhost, :committed]
        when 'abort' then @responseq <= [localhost, :aborted]
        else p 'weird 2PC msg received at client'
      end
    end
  end
end
