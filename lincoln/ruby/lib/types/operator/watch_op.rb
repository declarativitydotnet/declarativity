require 'lib/types/operator/operator'
class WatchOp < Operator
  class Modifier
    NONE=0 
    TRACE=1
    ADD=2
    ERASE=3
    INSERT=4
    DELETE=5
    RECEIVE=6
    SEND=7
  end
  @@modifiers = Hash.new

  @@modifiers[:t] = Modifier::TRACE
  @@modifiers[:a] = Modifier::ADD
  @@modifiers[:e] = Modifier::ERASE
  @@modifiers[:i] = Modifier::INSERT
  @@modifiers[:d] = Modifier::DELETE
  @@modifiers[:r] = Modifier::RECEIVE
  @@modifiers[:s] = Modifier::SEND
  
  def WatchOp.modifiers(s)
    @@modifiers[s]
  end


  def initialize(context, program, rule, name, modifier) 
    new_prnms(context,program,rule,name,modifier,STDERR)
    super(context, program, rule)
  end

  def new_prnms(context, program, rule, name, modifier, stream)
    @name = name
    @modifier = modifier
    @stream = stream
  end

  def evaluate(tuples)
#    require 'ruby-debug'; debugger
    return tuples if tuples.size == 0

    header = "Program " + @program.to_s + " [CLOCK " + @context.clock.current.to_s + "] " + @modifier.to_s + ": " + @name.to_s

    header += " Rule " + @rule.to_s unless @rule.nil?
    header += "\n\tSCHEMA: " + tuples.tups[0].schema.to_s

    puts(header)
#    stream.puts(header)
    tuples.each do |tuple|
#      stream.puts("\t" + tuple)
      puts("\t" + tuple.to_s)
    end
    return tuples
  end

  # weird
  def requires
    return Hash.new
  end

  def schema
    return nil
  end

  def to_s
    return "Watch " + @modifier.to_s + ": " + @name.to_s
  end
end
