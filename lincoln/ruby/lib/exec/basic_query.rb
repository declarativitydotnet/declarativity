require 'lib/exec/query'
require 'lib/types/exception/dataflow_runtime_exception'
class BasicQuery < Query

  def initialize(context, program, rule, isPublic, isDelete, event, head, body)
    @aggregation = nil
    require 'ruby-debug'; debugger if body.nil?
    @body        = body
    super(context, program, rule, isPublic, isDelete, event, head)
    @context = context
  end

  def to_s
    String   query = "Basic Query Rule " + rule + 
    ": input " + input.to_s
#    require 'ruby-debug'; debugger
    @body.each do |oper| 
      query += " -> " + oper.to_s
    end
    query += " -> " + output().to_s
    return query
  end

  def evaluate(inval)
    print "running query \"#{rule.to_s}(#{inval.name.to_s})\"\n"    
    if (@input.name != inval.name) then
      raise DataflowRuntimeException, "Query expects input " + @input.name.to_s + 
      " but got input tuples " + inval.name.to_s
    end

    tuples = TupleSet.new(@input.name)
    inval.each do |tuple| 
      require 'ruby-debug'; debugger if tuple.size != @input.schema.size
      tuple = tuple.clone
      tuple.schema = @input.schema.clone
      tuples << (tuple)
    end

#    require 'ruby-debug'; debugger if rule == 'q1_rule'
    @body.each do |oper| 
      # PUT IN SOME CHECK HERE THAT OPER SCHEMA VARIABLES NOT REPLICATED!
      # oper.schema.variables.each do |v|
      tuples = oper.evaluate(tuples)
    end
 #   require 'ruby-debug'; debugger if output.name.to_s == 'runtime::insertionQueue' and tuples.size > 0
    print "    produced #{tuples.size.to_s} tups in #{output.name.to_s}:\n       #{tuples.tups.to_s}\n"
    puts if tuples.tups.size > 0 
    return tuples
  end
end
