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

  def set(context, program)
    @head.set(context, program, @name, 0)
    @body.each_with_index { |b, i| b.set(context, program, @name, i+1) }
    context.catalog.table(RuleTable.table_name).force(Tuple.new(program, @name, @isPublic, @isDelete, self))
  end

  def query(context, periodics)
    # First search for an event predicate.
    event   = nil
    function = nil
    body.each do |term|
      #  	  require 'ruby-debug'; debugger if name == 'tyson' 
      if (term.class <= Predicate)
        table = context.catalog.table(term.name)
        if table.nil?
          print context.catalog.to_s
#          require 'ruby-debug'; debugger
          raise "Table " + term.name.to_s + " not found in catalog" 
        end
        if (table.table_type == Table::TableType::EVENT || term.event != Predicate::Event::NONE) 
          if (!event.nil?)  
            require 'ruby-debug'; debugger				  
            raise PlannerException, "Multiple event predicates in rule " + name.to_s + 
            " location " + term.location.to_s
          end
          # Plan a query with this event predicate as input.
          event = term
        end
        if (event.class <= Function)
          event.event(Predicate.Event::INSERT)
        end
      end
    end

    if (!event.nil?)
#      puts "---- EVENT FOR RULE #{name.to_s} IS #{event.to_s} ----"
      return query_stuff(context, periodics, head, event, body)
    else
#      puts "---- NO EVENT FOR RULE #{name.to_s} ---- "
      return mviewQuery(context, head, body)
    end    
  end

  def mviewQuery(context, head, body)
    queries = Array.new
    body.each do |term1|
      if ((!(term1.class <= Predicate)) or term1.notin)
        next
      end
      queries += query_stuff(context, nil, head, term1, body)
    end
    return queries 
  end

  def localize(context, periodics, head, event, body)
    queries = Array.new

    # /* Group all predicate terms by the location variable. */
    groupByLocation = Hash.new
    remainder = Array.new
    body.each do |t|
      if (t.class <= Predicate)
        p = t
        loc = p.locationVariable.name
        if (!groupByLocation.has_key(loc))
          groupByLocation[loc] = Array.new
        end
        groupByLocation[loc] << p
      else
        remainder << t
      end
    end

    # /* Create the set of localized rules. */
    # /* Grab the location variable representing from the event for this 
    #  * rule group. The orginal event will be the first group. Subsequent 
    #  * groups will be triggered off the intermediate event predicates
    #  * created during the localization process. The final group will 
    #  * project onto the orginal head predicate. */
    while groupByLocation.size > 0 do
      location           = event.locationVariable.name
      intermediateSchema = event.schema
      intermediateName   = @name + "_intermediate_" + event.name.name

      # /* Get the set of predicates in this location group. */
      intermediateBody = groupByLocation[location]
      groupByLocation.delete(location)

      if (groupByLocation.size > 0)
        intermediateBody.each {|t| intermediateSchema = intermediateSchema.join(t.schema)} 

        # /* Turn off location variable(s) in intermediate schema. */
        intermediateSchema.variables.each {|v| v.loc(false)}

        # /* Locate the next location variable from remaining groups. 
        #  * The location variable must appear in the schema of the intermediate
        #  * schema (That is we need to know its value). */
        groupByLocation.keySet.each do |loc|
          var = intermediateSchema.variable(loc)
          if (!var.nil?)
            var.loc(true) # Make this the location variable.
            break
          end
        end

        intermediateEvent = EventTable.new(TableName.new(@program, intermediateName), TypeList.new(intermediateSchema.types))
        context.catalog.register(intermediateEvent)
        intermediate = Predicate.new(false, intermediateEvent.name, Predicate.Event::NONE, intermediateSchema)
        intermediate.program  = event.program
        intermediate.rule     = event.rule
        intermediate.position = 0

        # /* Create a query with the intermediate as the head predicate. */
        queries.addAll(query_stuff(context, periodics, intermediate, event, intermediateBody))

        # /* the intermediate predicate will be the event predicate in the next (localized) rule. */
        event = intermediate;
      else
        # /* This is the final group that projects back to the orginal head. */
        intermediateBody.addAll(remainder) # Tack on any remainder terms (e.g., selections, assignments).
        queries.addAll(query_stuff(context, periodics, head, event, intermediateBody))
      end
    end
    return queries
  end

  def query_stuff(context, periodics, head, event, body)
    queries = Array.new
    operators = Array.new

    loc = event.locationVariable
    body.each do |t|
      if t.class <= Predicate
        if loc.nil?
          raise "Can't mix location variables in a local rule!" if !t.locationVariable.nil?
        elsif !(loc == p.locationVariable)
          return localize(context, periodics, head, event, body)
        end
      end
    end

    if (event.name.name == "periodic") && !(event.name.scope == Table.GLOBALSCOPE)
      period = ttl = 1
      start = count = 0
      program = @program

      periodval = event.argument(Periodic.Field::PERIOD).value
      period = periodval if periodval.class <= Value
      ttlval    = event.argument(Periodic.Field::TTL).value
      ttl = tllval if tllval.class <= Value
      timeval    = event.argument(Periodic.Field::TIME).value
      start = timeval if timeval.class <= Value
      countval  = event.argument(Periodic.Field::COUNT).value
      count = countval if countval.class <= Value
      progval    = event.argument(Periodic.Field::PROGRAM).value
      program = progval if progval.class <= Value

      identifier = Runtime.idgen.to_s
      periodics << Tuple.new(identifier, period, ttl, start, count, program)

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

      efilter = EventFilter.new(context, event, periodicFilter)
      operators << efilter
    else
      #			  require 'ruby-debug'; debugger
      efilter = EventFilter.new(context, event)
      if (efilter.filters > 0)
        operators << efilter
      end
    end

    watch = context.catalog.table(WatchTable.table_name)
    if !(watch.watched(@program, event.name, WatchOp::Modifier::RECEIVE).nil?) then
      operators << Watch.new(context, @program, name, event.name, WatchOp::Modifier::RECEIVE)
    end

    operators << event.operator(context, event.schema.clone) if (event.class <= Function)

    schema = event.schema.clone
    body.each do |term| 
      if !(term == event) 
        oper = term.operator(context, schema)
        operators << oper
        schema = oper.schema
      end
    end

    operators << Projection.new(context, @head)

    if !(watch.watched(@program, @head.name, WatchOp::Modifier::SEND).nil?)
      operators << Watch.new(context, @program, name, @head.name, WatchOp::Modifier::SEND)
    end

    headLoc = head.locationVariable
    eventLoc = event.locationVariable
    operators << RemoteBuffer.new(context,head,isDelete) if (!headLoc.nil? and !eventLoc.nil? and headLoc != eventLoc)

    qry = []
#    require 'ruby-debug'; debugger
    qry << BasicQuery.new(context,@program,@name,@isPublic,@isDelete,event,@head,operators)
    return qry
  end
end
