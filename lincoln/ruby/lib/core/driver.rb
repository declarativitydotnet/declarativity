require 'lib/types/table/table_function'
require 'lib/types/basic/tuple_set'
require 'lib/types/table/aggregation_table'
require 'lib/types/table/event_table'
require 'lib/types/operator/watch_op'
require 'monitor'

class Driver < Monitor
  Infinity = 1.0/0
  @@threads = []

  class Evaluate < TableFunction
    class UpdateState
      def initialize
        @insertions = Hash.new
        @deletions  = Hash.new
      end
    end

    class EvalState
      def initialize(time, program, name)
        @time    = time
        @program = program
        @name    = name
        @insertions = TupleSet.new(name)
        @deletions = TupleSet.new(name)
      end

      attr_reader :time, :program, :name
      attr_accessor :insertions, :deletions

      def hash
        to_s.hash
      end

      def ==(o) 
        return (o <= EvalState && (to_s == o.to_s))
      end

      def to_s
        return @program + ":" + @time.to_s + ":" +  @name.to_s
      end
    end

    class Field
      TIME=0 
      PROGRAM=1
      TABLENAME=2
      INSERTIONS=3
      DELETIONS=4
    end
    @@SCHEMA =  [Numeric,String,TableName,TupleSet,TupleSet]
    # Long.class,       // Evaluation time
    # String.class,     // Program name
    # TableName.class,  // Table name
    # TupleSet.class,   // Insertion tuple set
    # TupleSet.class    // Deletions tuple set

    def initialize 
      super("evaluate", TypeList.new(@@SCHEMA))
    end

    def insert(tuples, conflicts)
      evaluations = Hash.new

      tuples.each do |tuple| 
        time    = tuple.value(Field::TIME)
        program = tuple.value(Field::PROGRAM)
        name = tuple.value(Field::TABLENAME)
        unless name.class <= String or name.class <= TableName
          require 'ruby-debug'; debugger
        end
        state = EvalState.new(time, program, name)

        if !(evaluations.has_key? state) 
          evaluations[state] = state
        else 
          state = evaluations[state]
        end

        insertions  = tuple.value(Field::INSERTIONS)
        deletions   = tuple.value(Field::DELETIONS)

        state.insertions.addAll(insertions)
        state.deletions.addAll(deletions)
      end

      delta = TupleSet.new(name)

      evaluations.values.each do |state|
        if state.name.class != TableName
          require 'ruby-debug'; debugger
        end
        results = evaluate(state.time, System.program(state.program), state.name, state.insertions, state.deletions)
        results.each {|t| delta << t}
      end
      return delta
    end

    def evaluate(time, program, name, insertions, deletions) 
      continuations = Hash.new
      table = Table.find_table(name)
      if table.nil?
        require 'ruby-debug'; debugger
        raise "Table #{name} not found"
      end
      watchAdd    = Compiler.watch.watched(program.name, name, WatchOp::Modifier::ADD)
      watchInsert = Compiler.watch.watched(program.name, name, WatchOp::Modifier::INSERT)
      watchRemove = Compiler.watch.watched(program.name, name, WatchOp::Modifier::ERASE)
      watchDelete = Compiler.watch.watched(program.name, name, WatchOp::Modifier::DELETE)

      begin  
        watchAdd.evaluate(insertions) unless watchAdd.nil?

        insertions = table.insert(insertions, deletions)
        break if insertions.size == 0

        watchInsert.evaluate(insertions) unless watchInsert.nil?

        querySet = program.get_queries(insertions.name)
        break if querySet.nil?

        delta = TupleSet.new(insertions.name)
        querySet.each do |query|
          if query.event != Table::Event::DELETE
            puts("\t\tRUN QUERY " + query.rule.to_s + " input " + insertions.tups.to_s)
            #            puts("\t\t\t Plan #{query.to_s}")
            result = query.evaluate(insertions) 
            puts("\t\tQUERY " + query.rule.to_s + " result " + result.tups.to_s)
            # require 'ruby-debug'; debugger
            if result.size == 0
              next
            elsif result.name == insertions.name
              if query.isDelete
                deletions.addAll(result)
              else
                delta.addAll(result)
              end
            else 
              if query.isDelete
                continuation(continuations, time, program.name, Table::Event::DELETE, result)
              else 
                continuation(continuations, time, program.name, Table::Event::INSERT, result)
              end
            end
          end
        end
        insertions = delta
      end while insertions.size > 0

      if !(table.class <= AggregationTable)
        while deletions.size > 0
          if table.table_type == Table::TableType::TABLE
            watchRemove.evaluate(deletions) unless watchRemove.nil?
            deletions = table.delete(deletions)
            watchDelete.evaluate(deletions) unless watchDelete.nil?
          else
            raise "Can't delete tuples from non table type"
            exit
          end
          delta = TupleSet.new(deletions.name)
          queries = program.get_queries(delta.name)
          if not queries.nil?
            queries.each do |query|
              output = Table.find_table(query.output.name)
              if !(output.class <= EventTable) and query.event != Table::Event::INSERT
                result = query.evaluate(deletions)
                if result.size == 0
                  next
                elsif not result.name.equals(deletions.name)
                  Table t = Table.table(result.name)
                  if t.table_type == Table::TableType::TABLE
                    continuation(continuations, time, program.name, Table::Event::DELETE, result)
                  end
                else 
                  delta.addAll(result)
                end
              end
            end
            deletions = delta
          end
        end

        delta = TupleSet.new(name)
        continuations.values.each { |c| delta << c }
        puts("==================== RESULT " + name.to_s + ": " + delta.to_s + "\n\n")
        return delta
      end
    end

    def continuation(continuations, time, program, event, result) 
      key = program.to_s + "." + result.name.to_s

      if not continuations.has_key?(key)
        tuple = Tuple.new(time, program, result.name, TupleSet.new(result.name), TupleSet.new(result.name))
        continuations[key] = tuple
      end

      if event == Table::Event::INSERT 
        insertions = continuations[key].value(Field::INSERTIONS)
        insertions.addAll(result)
      else 
        deletions = continuations[key].value(Field::DELETIONS)
        deletions.addAll(result)
      end
    end
  end

  class Task 
    # public TupleSet tuples
    # 
    # public String program
  end

  # /* Tasks that the driver needs to execute during the next clock. */
  # private List<Task> tasks
  # 
  # /** The schedule queue. */
  # private Program runtime
  # 
  # private Schedule schedule
  # 
  # private Periodic periodic
  # 
  # private Clock clock

  def initialize(runtime, schedule, periodic, clock) # Driver
    @tasks = Array.new
    @runtime = runtime
    @schedule = schedule
    @periodic = periodic
    @clock = clock
    super() # initialize the monitor stuff
  end

  def task(t) 
    @tasks << t
  end

  def run 
    @@threads << Thread.new() do
      while (true) 
        synchronize do
          min = Infinity

          if @schedule.cardinality > 0
            if min.class == String or @schedule.min.class == String
              require 'ruby-debug'; debugger
            end
            min = (min < @schedule.min ? min : @schedule.min)
          elsif @tasks.size > 0
            min = @clock.current + 1
          end

          if min < Infinity
            puts("============================ EVALUATE CLOCK[" + min.to_s + "] =============================")
            # @schedule.dump_to_tmp_csv
#            print "SCHEDULE: " + @schedule.to_s
            evaluate(@clock.time(min), @runtime.name)
            #            print @tasks.to_s
            @tasks.each { |t| evaluate(t.tuples, t.program) }
            @tasks.clear
            puts("============================ ========================== =============================")
          end
          #     try {      
          sleep(1)
          #     end catch (InterruptedException e)  end
        end
      end
    end
    @@threads.each {|t| t.join}
  end

  def evaluate(tuples, program) 
    evaluation = TupleSet.new(System.evaluator.name)
    evaluation << Tuple.new(@clock.current, program, tuples.name, tuples, TupleSet.new(tuples.name))
    # Evaluate until nothing left in this clock.
    while evaluation.size > 0
      evaluation = System.evaluator.insert(evaluation, nil)
    end
  end
end
