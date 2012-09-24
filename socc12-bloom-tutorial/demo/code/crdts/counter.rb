require 'rubygems'
require 'bud'

module BCounter
  state do
    interfaces :input, [:msg]

    channel :msg, [:@site, :content]
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

module QueryableBCounter
  include BCounter
  include QueryPreds

  bloom do
    query_resp <~ query{|q| [q.return_add, cnt.reveal]}
  end
end

module GuardedQueryableBCounter
  include BCounter
  include QueryPreds

  state do
    table :qguard, query.schema
  end

  bloom do
    qguard <= query
    query_resp <~ qguard{|q| [q.return_add, cnt.reveal]}
  end
end
