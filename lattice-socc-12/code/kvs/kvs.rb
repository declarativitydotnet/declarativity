require 'rubygems'
require 'bud'

require './lpair'

module KvsProtocol
  state do
    channel :kvput, [:req_id, :@addr] => [:key, :value, :client_addr]
    channel :kvput_response, [:req_id] => [:@addr, :replica_addr]
    channel :kvget, [:req_id, :@addr] => [:key, :client_addr]
    channel :kvget_response, [:req_id] => [:@addr, :value, :replica_addr]

    channel :kv_do_repl, [:@addr, :target_addr]
  end
end

# Simple KVS replica in which we just merge together all the proposed values for
# a given key. This is reasonable when the intent is to store a monotonically
# increasing lmap of keys.
class MergeMapKvsReplica
  include Bud
  include KvsProtocol

  state do
    lmap :kv_store
  end

  bloom do
    kv_store <= kvput {|c| {c.key => c.value}}
    kvput_response <~ kvput {|c| [c.req_id, c.client_addr, ip_port]}
    # XXX: if the key does not exist in the KVS, we want to return some bottom
    # value. For now, ignore this case.
    kvget_response <~ kvget {|c| [c.req_id, c.client_addr,
                                  kv_store.at(c.key), ip_port]}
  end
end

module KvsProtocolLogger
  bloom do
    stdio <~ kvput {|c| ["kvput: #{c.inspect} @ #{ip_port}"]}
    stdio <~ kvget {|c| ["kvget: #{c.inspect} @ #{ip_port}"]}
    stdio <~ kv_do_repl {|c| ["kv_do_repl: #{c.inspect} @ #{ip_port}"]}
  end
end

class ReplicatedKvsReplica < MergeMapKvsReplica
  state do
    channel :repl_propagate, [:@addr] => [:kv_store]
  end

  bloom do
    repl_propagate <~ kv_do_repl {|r| [r.target_addr, kv_store]}
    kv_store <= repl_propagate {|r| r.kv_store}
  end
end

class KvsClient
  include Bud
  include KvsProtocol

  def initialize(addr)
    @req_id = 0
    @addr = addr
    super()
  end

  # XXX: Probably not thread-safe.
  def read(key)
    req_id = make_req_id
    r = sync_callback(:kvget, [[req_id, @addr, key, ip_port]], :kvget_response)
    r.each do |t|
      return t.value if t[0] == req_id
    end
    raise
  end

  def write(key, val)
    req_id = make_req_id
    r = sync_callback(:kvput, [[req_id, @addr, key, val, ip_port]], :kvput_response)
    r.each do |t|
      return if t[0] == req_id
    end
    raise
  end

  # XXX: Probably not thread-safe
  def cause_repl(to)
    sync_do {
      kv_do_repl <~ [[@addr, to.ip_port]]
    }

    # To make it easier to provide a synchronous API, we assume that the
    # destination node (the target of the replication operator) is local.
    to.delta(:repl_propagate)
  end

  private
  def make_req_id
    @req_id += 1
    "#{ip_port}_#{@req_id}"
  end
end

class QuorumKvsClient
  include Bud
  include KvsProtocol

  state do
    table :quorum_reqs, [:req_id] => [:acks, :val]
    scratch :quorum_sizes, [:req_id] => [:sz, :val]
    scratch :quorum_tests, [:req_id] => [:ready, :val]
    scratch :got_quorum, [:req_id]
  end

  bloom do
    quorum_reqs  <= kvput_response {|r| [r.req_id, Bud::SetLattice.new([r.replica_addr]), nil]}
    quorum_reqs  <= kvget_response {|r| [r.req_id, Bud::SetLattice.new([r.replica_addr]), r.value]}
    quorum_sizes <= quorum_reqs {|o| [o.req_id, o.acks.size, o.val]}
    quorum_tests <= quorum_sizes {|s| [s.req_id, s.sz.gt_eq(@quorum_size), s.val]}
    got_quorum   <= quorum_tests {|t|
      t.ready.when_true {
        [t.req_id]
      }
    }
  end

  def initialize(quorum_size, *addrs)
    @quorum_size = quorum_size
    @req_id = 0
    @addrs = addrs
    super()
  end

  def write(key, val)
    req_id = make_req_id
    put_reqs = @addrs.map {|a| [req_id, a, key, val, ip_port]}
    r = sync_callback(:kvput, put_reqs, :got_quorum)
    r.each do |t|
      return if t[0] == req_id
    end
    raise
  end

  # XXX: not quorum-aware yet
  def read(key)
    req_id = make_req_id
    r = sync_callback(:kvget, [[req_id, @addrs.first, key, ip_port]],
                      :kvget_response)
    r.each do |t|
      return t.value if t[0] == req_id
    end
    raise
  end

  private
  def make_req_id
    @req_id += 1
    "#{ip_port}_#{@req_id}"
  end
end
