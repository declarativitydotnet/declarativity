require 'rubygems'
require 'bud'

require './kvs'

class QuorumKvsClient
  include Bud
  include KvsProtocol

  state do
    table :put_reqs, [:reqid] => [:acks]
    table :get_reqs, [:reqid] => [:acks, :val]
    scratch :w_quorum, [:reqid]
    scratch :r_quorum, [:reqid] => [:val]
  end

  bloom do
    put_reqs <= kvput_response {|r| [r.reqid, Bud::SetLattice.new([r.replica_addr])]}
    w_quorum <= put_reqs {|r|
      r.acks.size.gt_eq(@w_quorum_size).when_true {
        [r.reqid]
      }
    }

    get_reqs <= kvget_response {|r| [r.reqid,
                                     Bud::SetLattice.new([r.replica_addr]), r.val]}
    r_quorum <= get_reqs {|r|
      r.acks.size.gt_eq(@r_quorum_size).when_true {
        [r.reqid, r.val]
      }
    }
  end

  def initialize(put_list, get_list)
    @put_addrs = put_list
    @get_addrs = get_list
    @r_quorum_size = get_list.size
    @w_quorum_size = put_list.size
    @reqid = 0
    super()
  end

  def write(key, val)
    reqid = make_reqid
    put_reqs = @put_addrs.map {|a| [reqid, a, key, val, ip_port]}
    r = sync_callback(:kvput, put_reqs, :w_quorum)
    r.each {|t| return if t.reqid == reqid}
    raise
  end

  def read(key)
    reqid = make_reqid
    get_reqs = @get_addrs.map {|a| [reqid, a, key, ip_port]}
    r = sync_callback(:kvget, get_reqs, :r_quorum)
    r.each {|t| return t.val if t.reqid == reqid}
    raise
  end

  private
  def make_reqid
    @reqid += 1
    "#{ip_port}_#{@reqid}"
  end
end
