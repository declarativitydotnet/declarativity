require 'rubygems'
require 'bud'

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

  bloom do
    kv_store <= kvput {|c| {c.key => c.value}}
    kvget_response <~ kvget {|c| [c.reqid, c.client_addr, kv_store.at(c.key)]}
  end

  bloom :logging do
    stdio <~ kvput {|c| ["kvput: #{c.inspect} @ #{ip_port}"]}
    stdio <~ kvget {|c| ["kvget: #{c.inspect} @ #{ip_port}"]}
  end
end

# Design notes:
#
# Inspired by Dynamo. Each key is associated with a <vector-clock, value>
# pair. A read returns this pair.
#
# To issue a write, a client must supply a <vector-clock, value> pair. The
# vector-clock represents the most recent version of this key read by the
# client, if any (if the client is doing a "blind write", they can use the empty
# clock). The write is received by a replica, which _coordinates_ applying the
# write to the rest of the system. First, the coordinator increments its own
# position in the client-submitted vector clock. It then merges the resulting
# <vector-clock, value> pair into its own database, according to this logic:
# versions are preferred according to the partial order over VCs, and for
# incomparable VCs (concurrent versions) a user-supplied merge method is
# invoked; the resulting merged value is assigned the merged VC of the two input
# values. Since we do conflict resolution at the server side, each replica
# stores at most one <vector-clock, value> associated with any key (unlike in
# Dynamo).
#
# Writes are propagated between replicas via the same logic, except that a
# replica accepting a write from another replica does not bump its own position
# in the write's vector clock. Write progagation could either be done actively
# (coordinator doesn't return success to the client until W replicas have been
# written) or passively (replicas perioidically merge their databases).
class VectorClockKvsReplica
  include Bud
  include KvsProtocol

  state do
    lmap :kv_store
    scratch :put_client_vc, [:reqid] => [:key, :value]
    scratch :put_new_vc, put_client_vc.schema
  end

  bloom do
    # When we accept a write from the client, increment this node's position in
    # the vector clock associated with the write.

    put_client_vc <= kvput {|m| [m.reqid, m.key, m.value]}
    put_client_vc <= kvput {|m| [m.reqid, m.key,
                                 Bud::PairLattice.new([Bud::MapLattice.new({ip_port => Bud::MaxLattice.new(0)}), nil])]}
    put_new_vc <= put_client_vc {|p| [p.reqid, p.key, 

    # Map request IDs => submitted VCs
    in_progress_write <= kvput {|c| {c.reqid => c.value.fst}}
    # Default clock value is 0
    in_progress_write <= kvput {|c| {c.reqid => {ip_port => Bud::MaxLattice.new(0)}}}

    new_write_vc <= in_progress_write {|k,v| { k => v.at(ip_port) + 1 } }

    kvget_response <~ kvget {|c| [c.reqid, c.client_addr, kv_store.at(c.key)]}
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

# Simple test scenario
r = MergeMapKvsReplica.new
r.run_bg

c = KvsClient.new(r.ip_port)
c.run_bg
c.write('foo', Bud::MaxLattice.new(5))
c.write('foo', Bud::MaxLattice.new(10))
res = c.read('foo')
puts "res = #{res.inspect}"

r.stop_bg
c.stop_bg
