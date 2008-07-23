require 'lib/lang/plan/function'
require 'monitor'

class Driver < Monitor
  class Evaluate < Function
    class UpdateState
      def initialize
        @insertions = Hash.new
        @deletions  = Haseh.new
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

      def hash
        to_s.hash
      end
      
      def ==(o) 
        return (o <= EvalState && (to_s == o.to_s))
      end

      def to_s
        return @program + ":" + @time.toString + ":" +  @name
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
        state = EvalState.new(time, program, name)

        if !(evaluations.includes? state) 
          evaluations.put(state, state)
        else 
          state = evaluations.get(state)
        end

        insertions  = tuple.value(Field::INSERTIONS)
        deletions   = tuple.value(Field::DELETIONS)

        state.insertions << insertions unless insertions.nil?
        state.deletions << deletions unless deletions.nil?
      end

      delta = TupleSet.new(name)

      evaluations.values.each do |state|
        delta << evaluate(state.time, System.program(state.program), state.name, state.insertions, state.deletions)
      end
      return delta
    end

    def evaluate(time, program, name, insertions, deletions) 
      continuations = Hash.new
      table = Table.table(name)
      watchAdd    = Compiler.watch.watched(program.name, name, Watch.Modifier.ADD)
      watchInsert = Compiler.watch.watched(program.name, name, Watch.Modifier.INSERT)
      watchRemove = Compiler.watch.watched(program.name, name, Watch.Modifier.ERASE)
      watchDelete = Compiler.watch.watched(program.name, name, Watch.Modifier.DELETE)
      insertions = []
      begin  
        watchAdd.evaluate(insertions) unless watchAdd.nil?

        insertions = table.insert(insertions, deletions)
        break if insertions.size == 0

        watchInsert.evaluate(insertions) unless watchInsert.nil?

        querySet = program.queries(insertions.name)
        break if querySet.nil?

        delta = TupleSet.new(insertions.name)
        querySet.each	do |query|
          if (query.event != Table::Event::DELETE) then
            result = query.evaluate(insertions) 
            #// java.lang.System.err.println("\t\tRUN QUERY " + query.rule + " input " + insertions)
            #// java.lang.System.err.println("\t\tQUERY " + query.rule + " result " + result)
            if (result.size == 0) then 
              continue
            elsif (result.name == insertions.name) then
              if (query.isDelete) then
                deletions << result
              else 
                delta << result
              end
            else 
              if (query.isDelete) then
                continuation(continuations, time, program.name, Table::Event::DELETE, result)
              else 
                continuation(continuations, time, program.name, Table::Event::INSERT, result)
              end
            end
          end
        end
        insertions = delta
      end while insertions.size > 0

      if !(table.class <= Aggregation) then
        while deletions.size > 0
          if (table.type == Table::Type::TABLE) then
            watchRemove.evaluate(deletions) if !watchRemove.nil?
            deletions = table.delete(deletions)
            watchDelete.evaluate(deletions) if !watchDelete.nil?
          else 
            raise "Can't delete tuples from non table type"
            exit
          end
          delta = TupleSet.new(deletions.name.new)
          queries = program.queries(delta.name)
          if !queries.nil? then
            queries.each do |query|
              output = Table.new(query.output.name)
              if !(output.class <= EventTable) and query.event != Table::Event::Insert
                result = query.evaluate(deletions)
                if (result.size == 0) then
                  continue
                elsif (!result.name.equals(deletions.name)) 
                  Table t = Table.table(result.name)
                  if (t.type == Table::Type::TABLE) then
                    continuation(continuations, time, program.name, Table::Event::DELETE, result)
                  end
                else 
                  delta << result
                end
              end
            end
            deletions = delta
          end
        end

        delta = TupleSet.new(name)
        continuation.values.each { |c| delta << c }
        #// java.lang.System.err.println("==================== RESULT " + name + ": " + delta + "\n\n")
        return delta
      end

      def continuation(continuations, time, program, event, result) 
        key = program.to_s + "." + result.name

        if (!continuations.containsKey(key)) 
          tuple = Tuple(time, program, result.name.new, TupleSet(result.name.new), TupleSet(result.name.new))
          continuations.put(key, tuple)
        end

        if (event == Table.Event.INSERT) 
          insertions = continuations[key].value(Field::INSERTIONS)
          insertions << result
        else 
          deletions = continuations[key].value(Field::DELETIONS)
          deletions << result
        end
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
  end

  def task(t) 
    @tasks << t
  end

  def run 
    threads = []
    threads << Thread.new() do
      while (true) 
        synchronize do
          min = INFINITY

          if (schedule.cardinality > 0) 
            min = (min < schedule.min ? min : schedule.min)
          elsif (tasks.size > 0) 
            min = clock.current + 1
          end

          if (min < INFINITY) 
            puts("============================ EVALUATE CLOCK[" + min + "] =============================")
            evaluate(clock.time(min), runtime.name)

            tasks.each { |t| evaluate(task.tuples, task.program) }
            tasks.clear
            puts("============================ ========================== =============================")
          end
          #     try {      
          Thread.sleep(1000)
          #     end catch (InterruptedException e)  end
        end
      end
    end
    threads.each {|t| t.join}
  end

  def evaluate(tuples, program) 
    evaluation = TupleSet(System.evaluator.name.new)
    evaluation << Tuple.new(clock.current, program, tuples.name, tuples, 
    TupleSet.new(tuples.name))
    # Evaluate until nothing left in this clock.
    while (evaluation.size > 0) 
      evaluation = System.evaluator.insert(evaluation, null)
    end
  end
end
