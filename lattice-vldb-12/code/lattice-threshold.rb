require 'rubygems'
require 'bud'

QUORUM_SIZE = 5
RESULT_ADDR = "example.org"

class QuorumVoteL
  include Bud

  state do
    channel :vote_chn, [:@addr, :voter_id]
    channel :result_chn, [:@addr]
    lset    :votes
    lbool   :vote_done
  end

  bloom do
    votes      <= vote_chn {|v| v.voter_id}
    got_quorum <= votes.size.gt_eq(QUORUM_SIZE)
    result_chn <~ got_quorum.when_true { [RESULT_ADDR] }
  end
end
