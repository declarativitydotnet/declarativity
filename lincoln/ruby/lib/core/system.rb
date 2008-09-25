require 'lib/types/table/catalog'
require 'lib/exec/query'
require 'lib/core/schedule'
require 'lib/core/driver'
require 'lib/core/clock'
require 'lib/core/periodic'
require 'lib/core/log'
require 'lib/lang/compiler'
require 'open3'

class System
	#  temporary (pa)
	@@periodic = Periodic.new(@schedule)
	@@programs = Hash.new
	@@RUNTIME = "lib/core/runtime.olg"
		
	def init
		Table.init
		@@query      = QueryTable.new
		@@compile    = CompilerTable.new  ## add back once parser/compiler in place
		@@evaluator  = Driver::Evaluate.new  ## add back once driver in place
		@@schedule   = Schedule.new
		@@clock      = Clock.new("localhost")
		@@periodic   = Periodic.new(@schedule)
		@@log        = Log.new($stderr)
		@@programs   = Hash.new
	end
	
  def System.clock
	  @@clock
  end

  def System.evaluator
	  @@evaluator
  end

	def System.query
	  @@query
  end
  
  def System.periodic
    @@periodic
  end
  
  def System.program(name)
    @@programs.has_key?(name) ? @@programs[name] : nil
  end
	
  def System.install_program(name, program)
    @@programs[name] = program
  end
	
  def install(owner, file)
	##print "\t\tINSTALL #{file}\n"
    compilation = TupleSet.new(@@compile.name)
    compilation << Tuple.new(nil, owner, file, nil)
    schedule(compilation, "runtime")
  end
  
  def schedule(tuples, program)
    @driver.synchronize do
      tmpClass = Class.new(Driver::Task)
      tlam = lambda { return tuples }
      plam = lambda { return program }

      tmpClass.send :define_method, :tuples do
        return tlam.call
      end
      
      tmpClass.send :define_method, :program do
        return plam.call
      end
  
      @driver.task(tmpClass.new)
    end
  end  
  
  def bootstrap
    return if defined? @@initialized
    compiler = Compiler.new("system", @@RUNTIME);
    compiler.program.plan
    @@clock.insert(@@clock.time(0),nil)
			
    @driver = Driver.new(System.program("runtime"), @@schedule, @@periodic, @@clock);
			
    Compiler.files.each {|file| install("system", file)}
    Thread.new { @driver }
    @@initialized = TRUE
	end
	
  def main(args)
    bootstrap
    ##print "BOOTSTRAP done. should have table in scope\n"
    tabn = Table.find_table(TableName.new("runtime","priority"))
    #$catalog.tuples.each do |c|
    #	puts c
    #end
    ##print "FOUND (#{tabn})\n"
    args.each { |a| install("user", a) }
    @driver.run()
  end
end
