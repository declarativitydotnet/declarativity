class AggregationTable < Table	
  def initialize(context, predicate, the_type)
    raise ("Aggregation table key error") if pred_key(predicate).nil?
    super(predicate.name, the_type, pred_key(predicate), pred_types(predicate))
    @baseTuples = Hash.new
    @aggregateTuples = TupleSet.new(name)

    @aggregate = nil
    predicate.each do |arg|
      if arg.class <= Aggregate
        @aggregate = arg
        break
      end
    end
    raise "no agg found" if @aggregate == nil

    if (the_type == Table::TableType::TABLE) 
      @primary = HashIndex.new(context, self, @key, Index::Type::PRIMARY)
      @secondary = Hash.new
    end

  end

  def variable
    @aggregate
  end

  def pred_key(predicate)
    key = []
    predicate.each do |arg|
      unless arg.class <= Aggregate
        key << arg.position
      end
    end
    return Key.new(*key)
  end

  def pred_types(predicate)
    types = TypeList.new([])
    predicate.each do |arg|
      types << arg.expr_type
    end
    return types
  end

  def tuples
    @aggregateTuples.clone
  end

  def values
    vals = TupleSet.new(name)
    @baseTuples.values.each {|v| vals << v.result}
    return vals
  end
  
  def insert(insertions, deletions)
    if deletions.size > 0
      intersection = deletions.clone
    
      insertions = insertions - intersection
      deletions = deletions - intersection
      delta = delete(deletions)
      deletions.clear
      deletions.addAll(delta)
    end    

    insertions.each do |tuple|
      the_key = key.project(tuple)
      if !@baseTuples.has_key?(the_key)
 #       # require 'ruby-debug'; debugger
        @baseTuples[the_key] = AggregateFunction.function(@aggregate)
      end
      @baseTuples[the_key].insert(tuple)
    end
#    # require 'ruby-debug'; debugger
    delta = values - tuples
    delta = delta - deletions
    
    if table_type == Table::TableType::EVENT
      # require 'ruby-debug'; debugger if name.name == "strata"
      @baseTuples.clear
      @aggregateTuples.clear
      return delta
    end
    insertions = super(delta, deletions)
    return insertions
  end

  def insert_tup(tuple)
    @aggregateTuples << tuple
    return @aggregateTuples
  end

  def delete(deletions)
 #   # require 'ruby-debug'; debugger
    if table_type == Table::TableType::EVENT
      raise ("Aggregation table " + name.to_s + " is an event table!")
		end
		
		deletions.each do |tuple|
		  the_key = key.project(tuple)
		  if (@baseTuples.has_key?(key))
		    if (@baseTuples[key].tuples.size == 0)
		      @baseTuples.delete(key)
	      end
      end
    end
    
    delta = TupleSet.new(name)
    delta.addAll(tuples)
    delta.removeAll(values)
    return super(delta)
  end

  def delete_tup(tuple)
    # require 'ruby-debug'; debugger
    @aggregateTuples.delete(tuple)
    return @aggregateTuples
  end

  def cardinality 
    t = tuples
    return 0 if t.nil?
    return t.size
  end

  attr_reader :primary, :secondary
end
