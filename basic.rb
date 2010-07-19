require 'bloom'

class Basic < Bloom

  def state
    table :r, ['x', 'y']
    table :s, ['x', 'y']
    table :out, ['x', 'y1', 'y2']
  end
  
  def declaration
    strata[0] = rules {
      r << [1, 1]
      r << [2, 1]
      r << [2, 2]
      r << [3, 1]
      r << [3, 2]
      s << [1, 1]
      s << [2, 2]
      s << [3, 1]
      s << [3, 2]
      j = join(r,s)
      out <= j.map do |t1,t2|
        [t1.x, t1.y, t2.y] if t1.x == t2.x
      end
    }
  end
end

program = Basic.new('localhost', 12345)
program.tick
o = (program.out.map {|o| o}).sort
o.map {|o| puts o.inspect}
