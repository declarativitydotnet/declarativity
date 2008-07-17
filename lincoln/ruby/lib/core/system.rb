require 'lib/types/table/catalog'
require 'lib/types/table/index_table'
require 'lib/exec/query'
require 'lib/core/schedule'
require 'lib/core/clock'
require 'lib/core/periodic'
require 'lib/core/log'
require 'open3'

class System
	@@RUNTIME = "src/p2/core/runtime.olg"
	
	attr_reader :clock, :evaluator, :query, :periodic
	
	def init
		Table.init
		@query      = Query::QueryTable.new
#		@compile    = CompileTable.new  ## add back once parser/compiler in place
#		@evaluator  = Driver::Evaluate.new  ## add back once driver in place
		@schedule   = Schedule.new
		@clock      = Clock.new("localhost")
		@periodic   = Periodic.new(@schedule)
		@log        = Log.new($stderr)
		@programs   = Hash.new
	end
	
	def program(name)
		@programs.has_key?(name) ? programs[name] : nil
	end
	
	def program_np(name, program)
		@programs[name] = program
	end
	
  # public static void install(String owner, String file) {
  #   synchronized (driver) {
  #     final TupleSet compilation = new TupleSet(compile.name());
  #     compilation.add(new Tuple(null, owner, file, null));
  #     driver.task(new Driver.Task() {
  #       public TupleSet tuples() {
  #         return compilation;
  #       }
  #     });
  #   }
  # }	
  def bootstrap
		compiler = Compiler.new("system", RUNTIME);
		compiler.program.plan
		clock.insert(clock.time(0), nil)
			
		driver = Driver.new(program("runtime"), schedule, periodic, clock);
			
		Compiler.FILES.each do |file|
			compilation = TupleSet.new(@compile.name)
			compilation << Tuple.new(null, "system", file, null)
			driver.evaluate(compilation);
		end
	end
	
	def main(args)
		init
		bootstrap
		args.each_with_index { |a, i| install("user", args[i]) }
		driver.run()
	end
end
