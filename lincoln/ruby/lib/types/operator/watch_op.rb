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
	
	
	def initialize(program, rule, name, modifier) 
		super(program, rule)
		@name = name
		@modifier = modifier
		@stream = STDERR
	end
	
	def new_prnms(program, rule, name, modifier, stream)
		super(program, rule)
		@name = name
		@modifier = modifier
		@stream = stream
	end

  def evaluate(tuples)
		return tuples if tuples.size == 0
		
		header = "Program " + program.to_s + " [CLOCK " + System.clock.current + "] " + Modifier::to_s + ": " + name
				        
		header += " Rule " + rule unless @rule.nil?
		header += "\n\tSCHEMA: " + tuples.iterator.next.schema
		
		stream.puts(header)
		tuples.each do |tuple|
			stream.puts("\t" + tuple)
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
		return "Watch " + modifier + ": " + name
	end
end
