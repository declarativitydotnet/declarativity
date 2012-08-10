require "rubygems"
require "bud"

class CreditOnlyLedger
  include Bud

  state do
    interface input, :credit_op, [:acct_id] => [:amt]

    # map from account ID => lpset of <amount>
    lmap :ledger
    # map from account ID => lmax
    lmap :balance
    # map from account ID => lbool
    lmap :liquid
  end

  bloom do
    # apply credits to the ledger
    ledger <= credit_op {|c| {c.acct_id => Bud::PositiveSetLattice.new(c.cmt)}}

    # compute account liquidity
    balance <= ledger.apply_morph(:pos_sum)
    liquid  <= balance.apply_morph(:gt_eq, 0)
  end
end

class CompleteLedger
  include Bud

  state do
    interface input, :credit_op, [:acct_id] => [:amt]
    interface input, :debit_op,  [:acct_id] => [:amt]

    # map from account ID => lpset of <amount>
    lmap :c_ledger
    lmap :d_ledger

    # map from account ID => lmax
    lmap :credit_sum
    lmap :debit_sum

    scratch :account, [:id]
    # indicates whether a given account is currently liquid; changes
    # non-monotonically
    scratch :liquid_status, [:acct_id] => [:liquid]
  end

  bloom do
    # keep credits and debits separate
    c_ledger <= credit_op {|c| {c.acct_id => Bud::PositiveSetLattice.new(c.cmt)}}
    d_ledger <= debit_op  {|d| {d.acct_id => Bud::PositiveSetLattice.new(d.cmt)}}

    c_balance <= c_ledger.apply_morph(:pos_sum)
    d_balance <= d_ledger.apply_morph(:pos_sum)

    account <= c_balance.key_set.to_collection
    liquid_status <= account {|a| [a.id, c_balance.at(a.id).reveal >= d_balance.at(a.id).reveal] }
  end
end
