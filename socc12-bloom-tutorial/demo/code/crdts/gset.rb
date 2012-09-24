require 'rubygems'
require 'bud'

module GSetProto
  state do
    interface input, :add_val, [:value]
    interface input, :del_val, [:value]
    interface output, :read_vals, [:value]
  end
end

module GSet
  include GSetProto
  state do
    table :lcl_set, [:value]
  end

  bloom :set_logic do
    lcl_set <= add_val
    read_vals <= lcl_set
    lcl_set <- del_val
  end
end

module AbstractDistributedGSet
  include GSetProto
  # underspecified
  #import GSet => :gs
  state do
    interfaces :input, [:server]
    channel :add_chan, [:@site, :value]
    table :server, [:site]
  end
  bloom do
    add_chan <~ (add_val * server).pairs{|a, s| [s.site, a.value]}
    gs.add_val <= add_chan{|a| [a.value]}
  end

  bloom :read_logic do
    read_vals <= gs.read_vals
  end
end

module DistributedGSet
  include GSetProto
  include AbstractDistributedGSet
  import GSet => :gs
end

module AbstractMutableDistributedGSet
  include AbstractDistributedGSet
  state do
    channel :del_chan, [:@site, :value]
  end
  bloom do
    del_chan <~ (del_val * server).pairs{|a, s| [s.site, s.value]}
    gs.del_val <= del_chan{|c| [c.value]}
  end
end

module MutableDistributedGSet
  include AbstractMutableDistributedGSet
  include DistributedGSet
end

module TwoPSetOne
  include GSetProto
  import GSet => :adds
  import GSet => :dels

  bloom do
    adds.add_val <= add_val
    dels.del_val <= del_val
    read_vals <= adds.read_vals.notin(dels.read_vals, :value => :value)
  end
end

module DistributedTwoPSetOne
  include GSetProto
  include AbstractMutableDistributedGSet
  import TwoPSetOne => :gs
end
