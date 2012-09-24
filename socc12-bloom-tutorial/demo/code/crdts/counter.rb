require 'rubygems'
require 'bud'

module CounterPreds
  state do
    interfaces :input, [:msg]
    channel :msg, [:@site, :content]
  end
end

module NaiveCounterOne
  include CounterPreds
  state do
    table :cnt, [:value]
  end
  bootstrap do
    cnt << [0]
  end
  bloom do
    cnt <+ (cnt * msg).lefts{|c| [c.value + 1]}
    cnt <- (cnt * msg).lefts
  end
end

module NaiveCounterTwo
  include CounterPreds
  state do
    table :msgs, [:content]
    scratch :cnt, [:cnt]
  end
  bloom do
    msgs <= msg{|m| [m.content]}
    cnt <= msgs.group([], count)
  end
end

module BCounter
  include CounterPreds
  state do
    lset :msgs
    lmax :cnt
  end

  bloom do
    msgs <= msg
    cnt <= msgs.size
  end
end

module QueryPreds
  state do
    interfaces :input, [:query]
    interfaces :output, [:query_resp]
    channel :query, [:@site, :return_add]
    channel :query_resp, [:@client, :data]
  end
end

module GuardedQueryPreds
  include QueryPreds
  state do
    table :qguard, query.schema
  end
  bloom do
    qguard <= query
  end
end

module QueryableBCounter
  include BCounter
  include QueryPreds

  bloom do
    query_resp <~ query{|q| [q.return_add, cnt.reveal]}
  end
end

module GuardedQueryableBCounter
  include BCounter
  include GuardedQueryPreds

  bloom do
    query_resp <~ qguard{|q| [q.return_add, cnt.reveal]}
  end
end


module QueryableNC1
  include NaiveCounterOne
  include QueryPreds
  bloom do
    query_resp <~ (query * cnt).pairs{|q, c| [q.return_add, c.cnt]}
  end
end

module GuardedQueryableNC1
  include NaiveCounterOne
  include GuardedQueryPreds
  bloom do
    query_resp <~ (qguard * cnt).pairs{|q, c| [q.return_add, c.cnt]}
  end
end

module GuardedQueryableNC2
  include NaiveCounterTwo
  include GuardedQueryPreds
  bloom do
    query_resp <~ (qguard * cnt).pairs{|q, c| [q.return_add, c.value]}
  end
end
