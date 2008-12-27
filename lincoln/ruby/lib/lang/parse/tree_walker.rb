# a general Treetop walker that fires off callbacks registered by the caller.
require 'benchmark'
class TreeWalker
  include Benchmark
  class Handler
    def initialize(runtime)	
      #@term = string
      #@func = func
      #@recurse = recurse
      @runtime = runtime
    end
    def semantic
      # not implemented
      raise "not implemented at base class"

    end
    def set_token(str)
      @token = str
    end
    def token
      return @token
    end
  end


  def initialize(tree)
    @tree = tree
    @hash = Hash.new
  end

  def add_handler(string,func,recurse)

    if @hash[string] then
      # print "hmm, already have a handler for this term: "+string+"\n"
    else
      func.set_token(string)
      @hash[string] = func
    end
  end

  def handlers
    @hash.each do |k|
      # puts k
      # puts @hash[k]
    end
  end

  def handle(elem)
    std_handle(elem)
  end
  
  def method_handle(elem)
    # this needs to be reversed: foreach method, probe hash.
    @hash.each do |k|
      begin 
        m = elem.method(k[0].to_sym)
      rescue NameError
        next
      end
      h = k[1]
      h.set_token(k[0])
      p = m.call
      
      sem = (defined? p.text_value) ? p.text_value : p.to_s
      # require 'ruby-debug'; debugger if sem == 'elements'
      h.semantic(sem,p)
      if (@verbose == 'v') then  
        #				# require 'ruby-debug'; debugger if sem =~ /TableName\./
        print k[0]+ ": ("+sem+")\n"
      end 
      # print "INTER: #{k[0]}\n"	
    end
  end

  def std_handle(elem)
    # this needs to be reversed: foreach method, probe hash.
    @hash.each do |k|
#      # require 'ruby-debug'; debugger
      meth = "elem."+k[0]
      stm = "defined? "+meth
      if (eval(stm)) then
#        p = eval(meth)
        p = elem.send k[0].to_sym
        h = k[1]
        h.set_token(k[0])

        sem = (defined? p.text_value) ? p.text_value : p.to_s
        h.semantic(sem,p)
        if (@verbose == 'v') then  
          #				  # require 'ruby-debug'; debugger if sem =~ /TableName\./
          print k[0]+ ": ("+sem+")\n"
        end 
        #print "INTER: #{k[0]}\n"	
      end 
    end
  end
  
  def handle_inter(elem)
#    # require 'ruby-debug'; debugger
    ms = (elem.methods - Object.methods) & @hash.keys
    if !ms[0].nil? 
      p = elem.send ms[0].to_sym
      h = @hash[ms[0]]
      h.set_token(ms[0])

      sem = (defined? p.text_value) ? p.text_value : p.to_s
      h.semantic(sem,p)
      if (@verbose == 'v') then  
        #				  # require 'ruby-debug'; debugger if sem =~ /TableName\./
        print ms[0]+ ": ("+sem+")\n"
      end 
      #print "INTER: #{ms[0]}\n"	
    end       
  end
  
  def handle_fast(elem)
    (elem.methods - Object.methods).each do |m|
      k = @hash[m.to_s]
      if !k.nil? then 
        #puts m.inspect
        p = eval("elem."+m.to_s)
        k.set_token(m.to_s)

        sem = (defined? p.text_value) ? p.text_value : p.to_s
        k.semantic(sem,p)
        if (@verbose == 'v') then  
          # # require 'ruby-debug'; debugger if sem =~ /TableName\./
          print m.to_s+ ": ("+sem+")\n"
        end 
        #print "INTER: #{m.to_s}\n"
      end
    end
  end
  def mywalk(node)
    if (defined? node.elements) then
      node.elements.each do |elem|
        # should be using handle_fast...
        ptr = handle(elem)
        ##ptr = handle_fast(elem)
        # depth first...?
        if (elem.nonterminal?) then
          mywalk(elem)	
        end
      end
    end
  end
  
  def mywalk_inter(node)
    if (defined? node.elements) then
      node.elements.each do |elem|
        # should be using handle_fast...
        ptr = handle_inter(elem)
        # depth first...?
        if (elem.nonterminal?) then
          mywalk_inter(elem)	
        end
      end
    end
  end
    
  
  def walk(verbose)
    @verbose = verbose
#    bm(1) do |test|
#      test.report("inter:") do
#        mywalk_inter(@tree)
#      end
#      test.report("std:") do
        mywalk(@tree)
#      end
#    end
  end


end




