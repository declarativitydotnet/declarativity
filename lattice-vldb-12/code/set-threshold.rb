require 'rubygems'
require 'bud'

NOTIFY_ADDR = "..."
NUM_VOTERS = 10

class UnanimousVote
  include Bud

  state do
    channel :input_chn, [:@addr, :vote_id]
    lset    :votes
    lbool   :vote_done
    channel :output_chn, [:@addr]
  end

  bloom do
    votes       <= input_chn {|c| c.vote_id}
    vote_done   <= votes.size.gt_eq(NUM_VOTERS)
    output_chan <~ vote_done.when_true { [NOTIFY_ADDR] }
  end
end

