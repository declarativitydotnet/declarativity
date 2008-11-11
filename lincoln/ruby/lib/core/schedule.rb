class Schedule < ObjectTable

  @@TABLENAME = TableName.new(GLOBALSCOPE, "schedule")
  @@PRIMARY_KEY = Key.new()

  class Field 
    TIME = 0
    PROGRAM = 1
    TABLENAME = 2
    INSERTIONS = 3
    DELETIONS = 4
  end

  @@SCHEMA = [Integer, String, TableName, TupleSet, TupleSet]
  # Long.class,      // Time
  # String.class,    // Program name
  # TableName.class, // Table name
  # TupleSet.class,  // Insertion tuple set
  # TupleSet.class   // Deletion tuple set

  def initialize(context)
    super(context, @@TABLENAME, @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
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
    print "+++++++++++++++++++ SCHEDULE: " + self.to_s + "\n"
    print "+++++++++++++++++++  --> inserting " + t.tups.size.to_s + ": " + t.tups.to_s + "\n"
    super(t, c)
  end
  
  def delete(tups)
#    require 'ruby-debug'; debugger
  print "+++++++++++++++++++ SCHEDULE: " + self.to_s + "\n"
  print "+++++++++++++++++++  --> deleting " + tups.size.to_s + ": " + tups.tups.to_s + "\n"
    super(tups)
  end
  
  def delete_tup(tup)
    super(tup)
  end
end
