require 'rubygems'
require 'bud'

require './lpair'

module KvsProtocol
  state do
    channel :kvput, [:reqid, :@addr] => [:key, :val, :client_addr]
    channel :kvput_response, [:reqid] => [:@addr, :replica_addr]
    channel :kvget, [:reqid, :@addr] => [:key, :client_addr]
    channel :kvget_response, [:reqid] => [:@addr, :val, :replica_addr]

    channel :kv_do_repl, [:@addr, :target_addr]
  end
end

# Simple KVS replica in which we just merge together all the proposed values for
# a given key. This is reasonable when the intent is to store a monotonically
# increasing lmap of keys.
class KvsReplica
  include Bud
  include KvsProtocol

  state do
    lmap :kv_store
  end

  bloom do
    kv_store <= kvput {|c| {c.key => c.val}}
    kvput_response <~ kvput {|c| [c.reqid, c.client_addr, ip_port]}
    kvget_response <~ kvget {|c| [c.reqid, c.client_addr,
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

class ReplicatedKvsReplica < KvsReplica
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
    @reqid = 0
    @addr = addr
    super()
  end

  def read(key)
    reqid = make_reqid
    r = sync_callback(:kvget, [[reqid, @addr, key, ip_port]], :kvget_response)
    r.each {|t| return t.val if t.reqid == reqid}
    raise
  end

  def write(key, val)
    reqid = make_reqid
    r = sync_callback(:kvput, [[reqid, @addr, key, val, ip_port]], :kvput_response)
    r.each {|t| return if t.reqid == reqid}
    raise
  end

  def cause_repl(to)
    sync_do { kv_do_repl <~ [[@addr, to.ip_port]] }

    # To make it easier to provide a synchronous API, we assume that the
    # destination node (the target of the replication operator) is local.
    to.delta(:repl_propagate)
  end

  private
  def make_reqid
    @reqid += 1
    "#{ip_port}_#{@reqid}"
  end
end

