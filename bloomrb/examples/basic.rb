require 'rubygems'
require 'lib/bloom'

class Basic < Bloom

  def state
    table :r, ['x', 'y']
    table :s, ['x', 'y']
    scratch :out, ['x', 'y1', 'y2']
    scratch :about, ['x', 'y1', 'y2']
  end
  
  def declaration
    strata[0] = rules {
      r << ['a', 1]
      r << ['b', 1]
      r << ['b', 2]
      r << ['c', 1]
      r << ['c', 2]
      s << ['a', 1]
      s << ['b', 2]
      s << ['c', 1]
      s << ['c', 2]
      j = join [r,s], {r.x => s.x}
      out <= j.map do |t1,t2|
        [t1.x, t1.y, t2.y]
      end
      k = join [r,s], {r.x => s.x, r.y => s.y}
      about <= k.map do |t1,t2|
        [t1.x, t1.y, t2.y]
      end
      
    }
  end
end

program = Basic.new('localhost', 12345)
program.tick
puts 'we should see 1 a, 2 b\'s and 4 c\'s'
o = (program.out.map {|o| o}).sort
o.map {|o| puts o.inspect}
puts 'we should next see 1 a, 1 b, and 2 c\'s'
a = (program.about.map {|b| b}).sort
a.map {|b| puts b.inspect}