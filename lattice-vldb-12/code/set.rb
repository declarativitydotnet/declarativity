class Bud::SetLattice < Bud::Lattice
  wrapper_name :lset

  def initialize(i=[])
    @v = i.uniq
  end

  def merge(i)
    self.class.new(@v | i.reveal)
  end

  morph :intersect do |i|
    self.class.new(@v & i.reveal)
  end

  morph :contains? do |i|
    Bud::BoolLattice.new(@v.member? i)
  end

  monotone :size do
    Bud::MaxLattice.new(@v.size)
  end
end
