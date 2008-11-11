require 'lib/core/runtime'
class Query
  include Comparable
	def initialize(context, program, rule, isPublic, isDelete, input, output)
		@program = program
		@rule = rule
		@isPublic = isPublic
		@isDelete = isDelete
		@event  = input.event()
		@input = input
		@output = output
    # catalog tuple generated in Runtime.initialize
    #	  context.catalog.table(QueryTable.table_name).force(Tuple.new(@program, @rule, @isPublic, @isDelete, @event.to_s, @input.name, @output.name, self))
  end	
	
	attr_reader :event, :program, :rule, :isDelete, :isPublic, :input, :output
	
	def to_s
	  throw "Query.to_s must be subclassed"
  end
	
	def <=>(q)
		return hash < q.hash ? -1 : (hash > q.hash ? 1 : 0)
	end
	
	def evaluate(input)
	  throw "Need to subclass Query.evaluate"
  end
end
