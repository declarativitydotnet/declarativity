class Schedule < ObjectTable

  @@PRIMARY_KEY = Key.new()

  class Field 
    TIME = 0
    PROGRAM = 1
    INSERTIONS = 2
    DELETIONS = 3
  end

  @@SCHEMA = [Integer, String, TableName, TupleSet, TupleSet]
  # Long.class,      // Time
  # String.class,    // Program name
  # TableName.class, // Table name
  # TupleSet.class,  // Insertion tuple set
  # TupleSet.class   // Deletion tuple set

  def initialize
    super(TableName.new(GLOBALSCOPE, "schedule"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
  end

  def min
    curmin = nil
    tuples.each do |tuple|
      curmin = (!curmin.nil? and curmin < (tuple.value(Field::TIME))) ? curmin : tuple.value(Field::TIME)
      if curmin.class == String
        require 'ruby-debug'; debugger
        print curmin
      end
    end
    return curmin
  end
  
  def insert(t, c)
    super(t, c)
  end
  
  def delete(tups)
    require 'ruby-debug'; debugger
    super(tups)
  end
  
  def delete_tup(tup)
    require 'ruby-debug'; debugger
    super(tup)
  end
end
