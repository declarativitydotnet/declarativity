class System {
	@@RUNTIME = "src/p2/core/runtime.olg"
	
	def init
		Table.init
		@query      = QueryTable.new
		@compile    = CompileTable.new
		@evaluator  = new Evaluate.new
		@schedule   = new Schedule.new
		@clock      = new Clock.new("localhost")
		@periodic   = new Periodic.new(schedule)
		@log        = new Log.new
		@programs   = Hash.new
	end
	
	def clock
		@clock
	end
	
	def evaluator
		@evaluator
	end
	
	def query()
		@query
	end
	
	def periodic
		@periodic
	end
	
	def program(name)
		@programs.has_key?(name) ? programs[name] : nil
	end
	
	def program(name, program)
		programs[name] = program
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
