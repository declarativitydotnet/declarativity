class AggregationTable < Table	
  def initialize(context, predicate, type)
    super(predicate.name, type, predicate.key, predicate.types)
    @tuples = Hash.new
    @aggregates = Hash.new
    if (type == Table::Type::TABLE) 
      @primary = HashIndex.new(context, self, key, Index::Type::PRIMARY)
      @secondary = Hash.new
    end

    predicate.each do |arg|
      if arg.class < Aggregate
        @aggregate = arg
        break
      end
    end
  end

  def variable
    @aggregate
  end

  def key(predicate)
    key = Array.new<Integer>
    predicate.each do |arg|
      unless arg.class < Aggregate
        key << arg.position
      end
    end
    return Key.new(key)
  end

  def types(predicate)
    types = TypeList.new
    predicate.each do |arg|
      types << arg.table_type
    end
    return types
  end

  def tuples
    current = TupleSet.new(name)
    unless @aggregates.nil?
      @aggregates.values.each do |aggregate|
        current << aggregate.result unless aggregate.result.nil?
      end
    end
    return current
  end

  def insert(insertions, deletions)
    deletions -= insertions

    previous = tuples

    # Perform aggregation.
    insertions.each { |tuple| insert(tuple) }

    if !deletions.nil? && deletions.size > 0
      delete(deletions)
      deletions.clear
    end

    current = tuples

    if !deletions.nil?
      deletions += previous    # Add all previous tuples
      deletions -= current     # Remove all current tuples
    end

    delta = TupleSet.new(name)
    delta += current # Add all current tuples
    delta -= previous # Remove those that existed before

    callbacks.each do |c|
      c.insertion(delta)
      c.deletion(deletions)
    end

    if type == Table::TableType::EVENT
      @tuples.clear
      @aggregates.clear
    end
    return delta
  end

  def delete(deletions)
    previous = tuples

    # Perform deletions
    deletions.each { |tup| delete(tup) } 

    current = tuples

    updates = TupleSet.new(name)
    updates += previous          # Add all previous tuples
    updates -= current           # Remove all current tuples

    delta = TupleSet.new(name)
    delta += current             # Add all current tuples
    delta -= previous            # Remove those that existed before

    callbacks.each do |c|
      c.insertion(delta)
      c.deletion(updates)
    end

    return delta
  end

  def delete_tup(tuple)
    thekey = key.project(tuple)
    group = tuples[thekey]
    return false if group.nil?
    group.remove(tuple)

    # Recompute aggregate of tuple group.
    function = @aggregates[thekey]
    previous = function.result
    function.reset
    group.each { |t| function.evaluate(t) }

    # Did deletion caused an aggregate change?
    if function.result.nil?
      @aggregates.remove(thekey)
      return true
    end
    return !function.result.equals(previous)
  end

  def insert_tup(tuple)
    thekey = key.project(tuple)
    if (!tuples.containsKey(thekey)) then tuples.put(thekey, TupleSet.new(name))
      tuples[thekey] << tuple

      if (!@aggregates.containsKey(thekey))
        #print "ok, calling function over a non-constructed aggregate\n"
        @aggregates.put(thekey, Aggregate.function(@aggregate))
      end
      @aggregates[thekey].evaluate(tuple)
      return true
    end
  end

  def cardinality 
    return @tuples.length
  end

  attr_reader :primary, :secondary
end
