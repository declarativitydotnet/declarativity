require 'lib/types/table/object_table'
require 'lib/lang/plan/term'
require 'lib/types/table/key'
class SelectionTerm < Term 	
  class SelectionTable < ObjectTable 
    @@PRIMARY_KEY = Key.new(0,1,2)

    class Field 
      PROGRAM = 0
      RULE = 1
      POSITION = 2
      OBJECT = 3
    end
    # Program name, Rule name, Term position, Selection Object
    @@SCHEMA = [String,	String, Integer, SelectionTerm]

    def initialize
      super(TableName.new(GLOBALSCOPE, "selection"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))
    end

    def insert(tuple)
      object = tuple.value(Field.OBJECT)
      if (object.nil?) then
        throw UpdateException, "Selection object null!"
      end
      object.program  = tuple.value(Field.PROGRAM);
      object.rule     = tuple.value(Field.RULE);
      object.position = tuple.value(Field.POSITION);
      return super.insert(tuple);
    end
  end

  def initialize(bool)
    super()
    @predicate = bool
  end

  def to_s
    return predicate.to_s
  end

  def requires
    return predicate.variables
  end

  def predicate
    return @predicate
  end

  def operator(input)
    return Selection.new(self, input)
  end

  def set(program, rule, position) 
    @@Program.selection.force(Tuple.new(program, rule, position, self))
  end
end
