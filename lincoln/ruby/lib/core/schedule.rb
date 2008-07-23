class Schedule < ObjectTable

  @@PRIMARY_KEY = Key.new()

  class Field 
    TIME = 1
    PROGRAM = 2
    INSERTIONS = 3
    DELETIONS = 4
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
    curmin = exp(2^Bignum-1)
    tuples.each do |tuple|
      curmin = curmin < (tuple.value(Field::TIME)) ? curmin : tuple.value(Field::TIME)
    end
    return curmin
  end
end
