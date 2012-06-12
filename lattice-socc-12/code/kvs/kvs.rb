require 'rubygems'
require 'bud'

require './lpair'

module KvsProtocol
  state do
    channel :kvput, [:reqid] => [:@addr, :key, :value]
    channel :kvget, [:reqid] => [:@addr, :key, :client_addr]
    channel :kvget_response, [:reqid] => [:@addr, :value]

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
    # XXX: if the key does not exist in the KVS, we want to return some bottom
    # value. For now, ignore this case.
    kvget_response <~ kvget {|c| [c.reqid, c.client_addr, kv_store.at(c.key)]}
  end
end

module KvsProtocolLogger
  bloom do
    stdio <~ kvput {|c| ["kvput: #{c.inspect} @ #{ip_port}"]}
    stdio <~ kvget {|c| ["kvget: #{c.inspect} @ #{ip_port}"]}
    stdio <~ kvget {|c| ["kv_do_repl: #{c.inspect} @ #{ip_port}"]}
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
    sync_do {
      kvput <~ [[make_req_id, @addr, key, val]]
    }
  end

  # To make it easier to provide a synchronous API, we assume that the
  # destination node (the target of the replication operator) is local.
  def cause_repl(to)
    sync_do {
      kv_do_repl <~ [[@addr, to.ip_port]]
    }

    # XXX: Probably not thread-safe
    to.delta(:repl_propagate)
  end

  private
  def make_req_id
    @req_id += 1
    "#{ip_port}_#{@req_id}"
  end
end
