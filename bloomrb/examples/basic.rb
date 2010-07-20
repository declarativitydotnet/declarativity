require 'rubygems'
require 'lib/bloom'

class Basic < Bloom

  def state
    table :r, ['x', 'y']
    table :s, ['x', 'y']
    table :out, ['x', 'y1', 'y2']
  end
  
  def declaration
    strata[0] = rules {
      r << ['a', 1]
      r << ['a', 2]
      r << ['b', 1]
      r << ['b', 2]
      r << ['c', 1]
      r << ['c', 2]
      s << ['a', 1]
      s << ['b', 2]
      s << ['c', 1]
      s << ['c', 2]
      j = join(r,s)
      out <= j.map do |t1,t2|
        [t1.x, t1.y, t2.y] if t1.x == t2.x
      end
    }
  end
end

program = Basic.new('localhost', 12345)
program.tick
puts 'we should see 1 a, 2 b\'s and 4 c\'s'
o = (program.out.map {|o| o}).sort
o.map {|o| puts o.inspect}
