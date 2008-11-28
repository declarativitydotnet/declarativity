require 'lib/types/table/catalog'
require 'lib/exec/query'
require 'lib/core/schedule'
require 'lib/core/driver'
require 'lib/core/clock'
require 'lib/core/periodic'
require 'lib/core/log'
require 'lib/lang/compiler'
require 'open3'

class Runtime
  include MonitorMixin
  @@RUNTIME = "lib/core/runtime.olg"
	
  @@idGenerator = 0
  
  def idgen
    idGenerator += 1
    return idGenerator
  end
  
  def initialize
    super
    @catalog = Table.init(self)
    Compiler.init_bootstrap(self)
		Compiler.init_catalog(self)

		@schedule = Schedule.new(self)
		@clock    = Clock.new(self, "localhost")
		@catalog.register(@schedule)
		@catalog.register(@clock)

#   QueryTable is registered as part of bootstrap schema.		
#		@catalog.register(QueryTable.new(self))
		@catalog.register(Periodic.new(self, @schedule))
		@catalog.register(Log.new(self, STDERR))

#		@network    = Network.new(self)
    @network    = nil
		@driver     = Driver.new(self, @schedule, @clock)
		@thread     = nil
  	@thread     = Thread.new {Thread.stop; @driver.run_driver}
#		@timer      = Timer.new("Timer", true)
    @timer      = nil
  end
  
	def shutdown
	  ## XXXXXXX
	  @driver.synchronize do
      @thread.exit
      @network.shutdown unless @network.nil?
    end
  end
  
  attr_reader :thread, :catalog, :network, :clock, :timer, :driver
    
  def program(name)
    program = catalog.table(ProgramTable.table_name).primary.lookup_vals(name)
	  return nil if (program.size == 0)
    # XXXXXX
		tuple = program.tups[0]
		return tuple.value(ProgramTable::Field::OBJECT)
  end
  
  def install(owner, file)
		compilation = TupleSet.new(CompilerTable.table_name)
		compilation << Tuple.new(nil, owner, file.to_s, nil)
#		require 'ruby-debug'; debugger
		schedule("runtime", CompilerTable.table_name, compilation, TupleSet.new(CompilerTable.table_name))
	end
	
	def uninstall(name)
		uninstall = TupleSet.new(TableName.new("compiler", "uninstall"), Tuple.new(name, true))
		schedule("compile", uninstall.name, uninstall, nil)
	end
	
  #  Schedule a set of insertion/deletion tuples.
  #  @param program The program that should execute the delta tuples.
  #  @param name The table name w.r.t. the tuple insertions/deletions.
  #  @param insertions The set of tuples to be inserted and deltas executed.
  #  @param deletions The set of tuples to be deleted and deltas executed.
	def schedule(program, name, insertions, deletions)
		@driver.synchronize do
			if (program == "runtime") 
			  task = Driver::Task.new
			  task.insertions = insertions
			  task.deletions = deletions
			  task.program = program
			  task.name = name
#		    print "XXX Tasking: #{name}, #{task.insertions.tups[0]}, #{task.deletions.tups[0]}"
  			@driver.task(task)
			else
				tuple = Tuple.new(clock.current + 1, program, name, insertions, deletions)
#		    print "XXX Scheduling: #{name}, #{task.insertions.tups[0]}, #{task.deletions.tups[0]}"
				schedule.force(tuple)
			end
			# XXXXXXXXXX
			@driver.cond_var.signal
		end
	end
	
   # * Creates a new runtime object that listens on the given network port.
   # * @param port The network port that this runtime listens on.
   # * @return A new runtime object.
   # * @throws P2RuntimeException If something went wrong during bootstrap.
	def Runtime.create(port)
		runtime = Runtime.new
    compiler = Compiler.new(runtime, "system", @@RUNTIME);
		compiler.the_program.plan
		runtime.driver.runtime = runtime.program("runtime")
		# XXXXXXXXXXXXX
#		runtime.network.install(port)
   runtime.thread.run
   # runtime.driver.run_driver
    
#    runtime.thread = Thread.new { runtime.driver.run_driver }

		Compiler.files.each { |file| runtime.install("system", file) }
		return runtime
	end
	
	def Runtime.main(*args)
		if (args.length < 2)
			raise ("Usage: runtime port program")
		end
		
		# Initialize the global Runtime
#		require 'ruby-debug'; debugger
		runtime = Runtime.create(args[0])
		[1..args.length].each do |i| 
#			url = URL.new("file", "", args[i]);
#			runtime.install("user", url);
			runtime.install("user", args[i]);
		end
		runtime.thread.join
	end
end
