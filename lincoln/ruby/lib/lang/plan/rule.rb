## Due to cyclic dependency yuckage,
# if you require this file, you will first 
# need to require lib/lang/compiler in the same file

#require 'lib/lang/compiler'
require 'lib/lang/plan/clause'
require 'lib/lang/plan/boolean'
require 'lib/types/operator/watch_op'
require 'lib/types/operator/projection'
require 'lib/lang/plan/aggregate'
require 'lib/lang/plan/object_from_catalog'
require 'lib/types/table/object_table'
require 'lib/types/operator/event_filter'
require 'lib/exec/basic_query'
require 'lib/types/exception/planner_exception'
require 'lib/lang/parse/schema'

class Rule < Clause
	attr_accessor :program, :name, :isPublic, :isDelete
	
  def initialize(location, name, isPublic, isDelete,  head, body)
		super(location)
		@name = name
		@isPublic = isPublic
		@isDelete = isDelete
		@head = head
		@body = body
		@aggregation = false
		head.each do |arg|
			if (arg.class <= Aggregate)
				# assertion: only 1 aggregate.
				raise "only 1 agg allowed" if @aggregation == true
				@aggregation = true
			end
		end
	end
	
	def to_s
		value = (@isPublic ? "public " : "") + @name + (@isDelete ? " delete " : " ") + head.to_s + " :- \n"
		(0..@body.length).each do |i|
			value += "\t" + @body[i].to_s
			value += (i + 1 < @body.length) ? ",\n" : ";\n"
		end
		return value
	end
	
	def <=>(o)
		return -1 if not (o <= Rule)
		
		other = o
		otherName = other.program + ":" + other.name
		myName    = @program + ":" + @name
		return (otherName <=> myName)
	end
	
	attr_reader :head, :body

	def set(program)
		@head.set(program, @name, 0)
		@body.each_with_index { |b, i| b.set(program, @name, i+1) }
		Compiler.rule.force(Tuple.new(program, @name, @isPublic, @isDelete, self))
	end

	def query(periodics)
		# First search for an event predicate.
		event   = nil
		function = nil
		body.each do |term|
			if (term.class <= Predicate)
        table = Table.find_table(term.name)
				if (table.table_type == Table::TableType::EVENT || term.event != Table::Event::NONE) 
					if (!event.nil?)  				  
						raise PlannerException, "Multiple event predicates in rule " + name.to_s + 
								                   " location " + term.location.to_s
					end
					# Plan a query with this event predicate as input.
					event = term
				end
			elsif (term.class < Function) then
				event = term.predicate
				event.event(Table.Event.INSERT)
			end
		end
		
		queries = Array.new
		if !event.nil? then
			operators = Array.new
			
			if (event.name.name == "periodic") && !(event.name.scope == Table.GLOBALSCOPE)  then
				period = event.argument(Periodic.Field::PERIOD).value
				ttl    = event.argument(Periodic.Field::TTL).value
				count  = event.argument(Periodic.Field::COUNT).value
				values = Array.new
				values << event.identifier
				(1..event.arguments).each {|i| values << event.argument(i).value }
				periodics << Tuple.new(values)
				
				identifier = event.identifier

        # set up a periodic filter by sending a lambda evaluate function to a TupleFunction
				doit = lambda do |t|
          return identifier == tuple.value(Periodic.Field::IDENTIFIER)
        end
        periodicFilter = Class.new(TupleFunction)

        periodicFilter.send :define_method, :evaluate do |tuple|
          return doit.call(tuple)
        end
        periodicFilter.send :define_method, :returnType do 
          return Boolean 
        end

				efilter = EventFilter.new(event, periodicFilter)
				operators << efilter
			else
				efilter = EventFilter.new(event)
				if (efilter.filters > 0)
					operators << efilter
				end
			end
			
			if !(Program.watch.watched(@program, event.name, WatchOp::Modifier::RECEIVE).nil?) then
				operators << Watch.new(@program, name, event.name, WatchOp::Modifier::RECEIVE)
			end
			
			if !(function.nil?) then
				schema = function.predicate.schema.clone
				operators << function.operator(schema)
				body.each do |term| 
					if !(term == function) then
						oper = term.operator(schema)
						operators << oper
						schema = oper.schema
					end
				end
			else 
				schema = event.schema.clone
				body.each do |term| 
					if !(term == event) 
						oper = term.operator(schema)
						operators << oper
						schema = oper.schema
					end
				end
			end
			
			operators << Projection.new(@head)
			
			if !(Program.watch.watched(@program, @head.name, WatchOp::Modifier::SEND).nil?)
				operators << Watch.new(@program, name, @head.name, WatchOp::Modifier::SEND)
			end
			
			queries << BasicQuery.new(@program, name, isPublic, isDelete, event, @head, operators)
		else 
			# Perform delta rewrite.
			eventPredicates = Array.new
			body.each do |term1|
				next unless term1.class <= Predicate
				next if (term1.notin)
				eventPredicates << term1.name
				
				delta = term1
				operators = Array.new
				efilter = EventFilter.new(delta)
				operators << efilter if (efilter.filters > 0) 

				if !(Program.watch.watched(@program, delta.name, WatchOp::Modifier::RECEIVE)).nil? then
					operators << Watch.new(@program, name, delta.name, WatchOp::Modifier::RECEIVE)
				end
				
				schema = delta.schema.clone
				body.each do |term2| 
					if !(term2 == delta) then
						oper = term2.operator(schema)
						operators << oper
						schema = oper.schema
					end
				end
				
				operators << Projection.new(@head)
				if (!(Program.watch.watched(@program, @head.name, WatchOp::Modifier::SEND).nil?)) then
					operators << Watch.new(@program, @name, @head.name, WatchOp::Modifier::SEND)
        end
				
				queries << BasicQuery.new(@program, @name, @isPublic, @isDelete, delta, @head, operators)
			end
			
		end
		return queries
	end
end
