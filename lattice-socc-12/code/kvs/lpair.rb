require 'rubygems'
require 'bud'

class PairLattice < Bud::Lattice
  wrapper_name :lpair

  def initialize(i=nil)
    unless i.nil?
      reject_input(i) unless i.length == 2
      reject_input(i) unless i.all? {|v| v.kind_of? Bud::Lattice}
    end
    @v = i
  end

  def merge(i)
    i_val = i.reveal
    return i if @v.nil?
    return self if i_val.nil?

    # Sanity check: if the first element is equal, the second element should be
    # as well
    raise if @v.first == i_val.first && @v.last != i_val.last

    # The lattice API does not currently include a way to tell if one lattice
    # value is \lt another value. Hence, we instead use the merge method as
    # follows: if a.merge(b) == a, then a \gt_eq b must hold.  Similarly, if
    # a.merge(b) == b, we have a \lt_eq b. If neither is the case, the two
    # values must be incomparable, so we fall back to merging the second field.
    merge_first = @v.first.merge(i_val.first)
    if merge_first == @v.first
      return self
    elsif merge_first == i_val.first
      return i
    else
      return self.class.new([merge_first, @v.last.merge(i_val.last)])
    end
  end

  morph :fst do
    @v.first unless @v.nil?
  end

  # Call "sym" (passing "args") on the first element of the pair; returns a pair
  # with the return value of sym() along with the unmodified second element of
  # the pair. This is akin to map, except we don't let the user code explicitly
  # manipulate the second element of the pair (since it might change
  # non-monotonically).
  morph :apply_fst do |sym, *args|
    raise Bud::Error unless RuleRewriter.is_monotone(sym)
    self.class.new([@v.first.send(sym, *args), @v.last]) unless @v.nil?
  end

  def snd
    @v.last unless @v.nil?
  end
end
