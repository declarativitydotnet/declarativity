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

    # The lattice API does not currently include a way to tell if one lattice
    # value is \lt another value. Hence, we instead use the merge method as
    # follows: if a.merge(b) == a, then a \gt_eq b must hold.  Similarly, if
    # a.merge(b) == b, we have a \lt_eq b. If neither is the case, the two
    # values must be incomparable, so we fall back to merging the second field.
    merge_first = @v.first.merge(i_val.first)
    if merge_first.reveal == @v.first.reveal
      return self
    elsif merge_first.reveal == i_val.first.reveal
      return i
    else
      return self.class.new([merge_first, @v.last.merge(i_val.last)])
    end
  end
end
