require 'rubygems'
require 'bud'

class PairLattice < Bud::Lattice
  wrapper_name :lpair

  def initialize(i=nil)
    unless i.nil?
      reject_input(i) unless (i.kind_of?(Enumerable) && i.length == 2)
      x, y = i
      reject_input(i) unless x.kind_of?(Comparable)
      reject_input(i) unless (x.kind_of?(Bud::Lattice) && y.kind_of?(Bud::Lattice))
    end
    @v = i
  end

  def merge(i)
    i_val = i.reveal
    return i if @v.nil?
    return self if i_val.nil?

    if @v[0] > i_val[0]
      return self
    elsif i_val[0] > @v[0]
      return i
    else
      return self.class.new(@v[0].merge(i_val[0]), @v[1].merge(i_val[1]))
    end
  end
end
