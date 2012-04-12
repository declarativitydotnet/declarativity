require 'rubygems'
require 'bud'

QUORUM_SIZE = 5
RESULT_ADDR = "example.org"

class QuorumVote
  include Bud

  state do
    channel :vote_chn, [:@addr, :voter_id]
    channel :result_chn, [:@addr]
    table   :votes, [:voter_id]
    scratch :cnt, [] => [:cnt]
  end

  bloom do
    votes      <= vote_chn {|v| [v.voter_id]}
    cnt        <= votes.group(nil, count(:voter_id))
    result_chn <~ cnt {|c| [RESULT_ADDR] if c >= QUORUM_SIZE}
  end
end
