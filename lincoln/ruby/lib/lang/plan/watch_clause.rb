require 'lib/lang/plan/clause'
require 'lib/types/table/object_table'
#require 'lib/types/operator/watch_op'
class WatchClause < Clause
	def initialize(location, name, modifier)
		super(location)
		@name = name
		@modifier = modifier
	end
	
	def to_s
		return "watch(" + name + ", " + modifier + ")."
	end

	def set(program)
		Compiler.watch.force(Tuple.new(program, name, modifier, Watch.new(program, null, name, modifier)))
	end
end
