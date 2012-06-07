require 'rubygems'
require 'bud'

module KvsProtocol
  state do
    channel :kvput, [:reqid] => [:@addr, :key, :value]
    channel :kvget, [:reqid] => [:@addr, :key, :client_addr]
    channel :kvget_response, [:reqid] => [:@addr, :value]
  end
end

class MergeMapKvsReplica
  include Bud
  include KvsProtocol

  state do
    lmap :kv_store
  end

  bloom do
    kv_store <= kvput {|c| {c.key => c.value}}
    kvget_response <~ kvget {|c| [c.reqid, c.client_addr, kv_store.at(c.key)]}
  end

  bloom :logging do
    stdio <~ kvput {|c| ["kvput: #{c.inspect} @ #{ip_port}"]}
    stdio <~ kvget {|c| ["kvget: #{c.inspect} @ #{ip_port}"]}
  end
end

class KvsClient
  include Bud
  include KvsProtocol

  def initialize
    @req_id = 0
    super
  end

  def read(key, addr)
    req_id = make_req_id
    r = sync_callback(:kvget, [[req_id, addr, key, ip_port]], :kvget_response)
    r.each do |t|
      if t[0] == req_id
        return t.value
      end
    end
    raise
  end

  def write(key, val, addr)
    sync_do {
      kvput <~ [[make_req_id, addr, key, val]]
    }
  end

  private
  def make_req_id
    @req_id += 1
    "#{ip_port}_#{@req_id}"
  end
end

# Simple test scenario
r = MergeMapKvsReplica.new
r.run_bg

c = KvsClient.new
c.run_bg
c.write('foo', Bud::MaxLattice.new(5), r.ip_port)
c.write('foo', Bud::MaxLattice.new(10), r.ip_port)
res = c.read('foo', r.ip_port)
puts "res = #{res.inspect}"

r.stop_bg
c.stop_bg
