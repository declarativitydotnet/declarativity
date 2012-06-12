require 'rubygems'
require 'bud'

require './lpair'

module KvsProtocol
  state do
    channel :kvput, [:reqid] => [:@addr, :key, :value]
    channel :kvget, [:reqid] => [:@addr, :key, :client_addr]
    channel :kvget_response, [:reqid] => [:@addr, :value]
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

  bloom :write do
    kv_store <= kvput {|c| {c.key => c.value}}
  end

  bloom :read do
    # XXX: if the key does not exist in the KVS, we want to return some bottom
    # value. For now, ignore this case.
    kvget_response <~ kvget {|c| [c.reqid, c.client_addr, kv_store.at(c.key)]}
  end

  # bloom :logging do
  #   stdio <~ kvput {|c| ["kvput: #{c.inspect} @ #{ip_port}"]}
  #   stdio <~ kvget {|c| ["kvget: #{c.inspect} @ #{ip_port}"]}
  # end
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
      if t[0] == req_id
        return t.value
      end
    end
    raise
  end

  def write(key, val)
    sync_do {
      kvput <~ [[make_req_id, @addr, key, val]]
    }
  end

  private
  def make_req_id
    @req_id += 1
    "#{ip_port}_#{@req_id}"
  end
end
