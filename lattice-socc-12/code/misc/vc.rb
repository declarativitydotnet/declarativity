require 'rubygems'
require 'bud'

class VectorClock
  include Bud

  state do
    lmap :my_vc
    lmap :next_vc
    interface input, :in_msg, [:addr, :payload] => [:clock]
    interface input, :out_msg, [:addr, :payload]
    interface output, :out_msg_vc, [:addr, :payload] => [:clock]
  end

  bootstrap do
    my_vc <= {ip_port => Bud::MaxLattice.new(0)}
  end

  bloom do
    next_vc <= my_vc
    next_vc <= out_msg { {ip_port => my_vc.at(ip_port) + 1} }
    next_vc <= in_msg  { {ip_port => my_vc.at(ip_port) + 1} }
    next_vc <= in_msg  {|m| m.clock}
    my_vc <+ next_vc

    out_msg_vc <= out_msg {|m| [m.addr, m.payload, next_vc]}
  end
end
