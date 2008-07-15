require 'lib/types/table/table'

class EventTable < Table
  def initialize(name, types)
    super(name, Event, 0, 0, nil, types)
  end
  
  def insert(tuples, deletions)
    return tuples
  end
  
  def delete(tuples)
    if tuples.class != TupleSet
      raise "Can't delete a single tuple from an EventTable"
    end
    return tuples
  end
    
  def primary
    nil
  end
  
  def secondary
    nil
  end
  
  def tuples
    nil
  end
  
  def cardinality
    0
  end
end