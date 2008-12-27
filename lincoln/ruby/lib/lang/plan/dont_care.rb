require 'lib/lang/plan/variable'
class DontCare < Variable 
  @@DONTCARE = "_"
  @@ids = 0

  def initialize(type, position, loc)
    super("DC" + @@ids.to_s, @type, position, loc);
    @@ids+= 1
  end

  def function()
    throw RuntimeError, "@position < 0" unless @position >= 0
    e_lam = lambda do |t|
      return t.values[@position]
    end

    r_lam = lambda do
      return @type
    end

    tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      e_lam.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      r_lam.call
    end
    return tmpClass.new
  end
end