require 'rubygems'
require 'bud'

module KVSProtocol
  state do
    interface input, :kvput, [:reqid] => [:key, :value]
    interface input, :kvget, [:reqid] => [:key]
    interface output, :kvget_respinse, [:reqid] => [:value]
  end
end
