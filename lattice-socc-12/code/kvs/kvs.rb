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
      return t.value if t.req_id == req_id
    end
    raise
  end

  def write(key, val)
    req_id = make_req_id
    r = sync_callback(:kvput, [[req_id, @addr, key, val, ip_port]], :kvput_response)
    r.each do |t|
      return if t.req_id == req_id
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
    table :put_reqs, [:req_id] => [:acks]
    table :get_reqs, [:req_id] => [:acks, :value]
    scratch :w_quorum, [:req_id]
    scratch :r_quorum, [:req_id] => [:value]
  end

  bloom do
    put_reqs <= kvput_response {|r| [r.req_id, Bud::SetLattice.new([r.replica_addr])]}
    w_quorum <= put_reqs {|r|
      r.acks.size.gt_eq(@w_quorum_size).when_true {
        [r.req_id]
      }
    }

    get_reqs <= kvget_response {|r| [r.req_id,
                                     Bud::SetLattice.new([r.replica_addr]), r.value]}
    r_quorum <= get_reqs {|r|
      r.acks.size.gt_eq(@r_quorum_size).when_true {
        [r.req_id, r.value]
      }
    }
  end

  def initialize(put_list, get_list)
    @put_addrs = put_list
    @get_addrs = get_list
    @r_quorum_size = get_list.size
    @w_quorum_size = put_list.size
    @req_id = 0
    super()
  end

  def write(key, val)
    req_id = make_req_id
    put_reqs = @put_addrs.map {|a| [req_id, a, key, val, ip_port]}
    r = sync_callback(:kvput, put_reqs, :w_quorum)
    r.each do |t|
      return if t.req_id == req_id
    end
    raise
  end

  def read(key)
    req_id = make_req_id
    get_reqs = @get_addrs.map {|a| [req_id, a, key, ip_port]}
    r = sync_callback(:kvget, get_reqs, :r_quorum)
    r.each do |t|
      return t.value if t.req_id == req_id
    end
    raise
  end

  private
  def make_req_id
    @req_id += 1
    "#{ip_port}_#{@req_id}"
  end
end
