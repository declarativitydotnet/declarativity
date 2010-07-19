require 'rubygems'
require 'enumerator'
require 'msgpack'
require 'eventmachine'
require 'socket'
require 'superators'

class Bloom
  attr_reader :strata, :bloomtime
  attr_accessor :connections
  attr_reader :tables # for debugging; remove me later

  def initialize(ip, port)
    @tables = {}
    @strata = []
    @channels = {}
    @bloomtime = 0
    @ip = ip
    @port = port.to_i
    $connections = {} if $connections.nil?
    $inbound = [] if $inbound.nil?

    @periodics = table :periodics, ['name'], ['ident', 'duration']
  end  


  ######## methods for controlling execution
  def run
    begin 
      EventMachine::run {
        EventMachine::start_server(@ip, @port, Server) { |con|
          con.bloom = self # pass this Bloom object into the connection
        }
        puts "running bloom server on #{@ip}, #{@port}"
        tick
      }
    end
  end

  def tick
    @bloomtime += 1
    # reset any schema stuff that isn't already there
    # state to be defined by the user program
    state

    receive_inbound

    # load the rules as a closure (will contain persistent tuples and new inbounds)
    # declaration to be provided by user program
    declaration

    @strata.each { |strat| stratum_fixpoint(strat) }
    @channels.each { |c| @tables[c[0]].flush }
    reset_periodics
  end

  # handle any inbound tuples off the wire and then clear
  def receive_inbound
    $inbound.each do |msg|
      tables[msg[0].to_sym] << msg[1]
    end
    $inbound = []
  end

  def stratum_fixpoint(strat)
    cnts = Hash.new
    @tables.each_key do |k| 
#      define_method(k.to_sym) { @tables[k] }
      eval "#{k.to_s} = @tables[#{k}]"
    end
    begin
      cnts = {}
      @tables.each_key{|k| cnts[k] = @tables[k].length}
      strat.call
    end while cnts.inject(0){|sum,t| sum + (@tables[t[0]].length - t[1])} > 0
  end

  def reset_periodics
    @periodics.each do |p| 
      if @tables[p.name].length > 0 then
        set_timer(p.name, p.ident, p.duration) 
        @tables[p.name] = scratch p.name, ['ident'], ['time']
      end
    end
  end


  ######## methods for registering collection types
  def table(name, keys=[], cols=[], persist = :table)
    # rule out tablenames that used reserved words
    reserved = defined?(name)
    if reserved == "method" and not @tables[name] then
      # first time registering table, check for method name reserved
      raise BloomError, "symbol :#{name} reserved, cannot be used as table name"
    end
    
    # check for previously-defined tables
    if @tables[name] then
      # check for consistent redefinition, and "tick" the table
      if @tables[name].keys != keys or @tables[name].cols != cols then
        raise BloomError, "create :#{name}, keys = #{keys.inspect}, cols = #{cols.inspect} \n \
        table :#{name} already defined as #{@tables[name].keys.inspect} #{@tables[name].cols.inspect}"
      end
      @tables[name].tick
    else # define table
      @tables[name] = case persist
        when :table then BloomTable.new(name, keys, cols)
        when :channel then BloomChannel.new(name, keys, cols)
        when :periodic then BloomPeriodic.new(name, keys, cols)
        when :scratch then BloomScratch.new(name, keys, cols)
        else raise BloomError, "unknown persistence model"
      end
      self.class.send :define_method, name do 
        @tables[name]
      end 
    end
    return @tables[name]
  end

  def scratch(name, keys=[], cols=[])
    table(name, keys, cols, :scratch)
  end

  def channel(name, locspec, keys=[], cols=[])
    t = table(name, keys, cols, :channel)
    t.locspec = locspec
    @channels[name] = locspec
  end

  def periodic(name, duration)
    table(name, ['ident'], ['time'], :periodic)
    unless @periodics.has_key? [name]
      retval = [name, gen_id, duration]
      @periodics << retval
      set_timer(name, retval[1], duration)
    else
      retval = @periodics.find([name]).first
    end
    return retval
  end

  def join(*rels)
    BloomJoin.new(*rels)
  end

  ######## ids and timers
  def gen_id
    Time.new.to_i.to_s << rand.to_s
  end

  def set_timer(name, id, secs)
    EventMachine::Timer.new(secs) do
      @tables[name] <+ [[id, Time.new.to_s]]
      tick
    end
  end

  ######## the collection types
  class BloomCollection
    include Enumerable

    attr_accessor :schema, :keys, :cols

    def initialize(name, keys, cols)
      @name = name
      @schema = keys+cols
      @keys = keys
      @storage = {}
      @pending = {}
      raise BloomError, "schema contains duplicate names" if schema.uniq.length < schema.length
      schema_accessors
    end

    def clone
      retval = BloomTable.new(keys, schema - keys)
      retval.storage = @storage.clone
      retval.pending = @pending.clone
      return retval
    end   

    def cols
      schema - keys
    end

    def tick
      @storage = @pending
      @pending = {}
    end
    
    # define methods to turn 'table.col' into a [table,col] pair
    # e.g. to support somethin like 
    #    j = join link, path, {link.to => path.from}
    def schema_accessors
      s = @schema
      m = Module.new do
        s.each_with_index do |c, i|
          define_method c.to_sym do
            [@name, i]
          end
        end
      end
      self.extend m  
    end

    # define methods to access tuple attributes by column name
   def tuple_accessors(t)
      s = @schema
      m = Module.new do
        s.each_with_index do |c, i|
          define_method c.to_sym do 
            t[i]
          end
        end
      end
      t.extend m
#      return t
    end

    def each
      @storage.each_key do |k|
        tup = (@storage[k] == true) ? k : (k + @storage[k])
        yield tuple_accessors(tup)
      end
    end

    def each_pending
      @pending.each_key do |k|
        k = tuple_accessors(k)
        if @pending[k] == true
          yield k
        else
          yield k + @pending[k]
        end
      end
    end

    def do_insert(o, store)
      return if o.nil? or o.length == 0
      keycols = keys.map{|k| o[schema.index(k)]}
      vals = (schema - keys).map{|v| o[schema.index(v)]}
      vals = true if vals.empty?
      if not store[keycols].nil? then
        raise KeyConstraintError, "Key conflict on insert" unless store[keycols].nil? or vals == store[keycols]
      end
      store[keycols] = vals unless store[keycols]
      return o
    end

    def insert(o)
      do_insert(o,@storage)
    end

    alias << insert

    def pending_insert(o)
      do_insert(o, @pending)
    end

    def merge(o)
      delta = o.map {|i| self.insert(i)}
      if self.schema.empty? and o.respond_to?(:schema) and not o.schema.empty? then 
        self.schema = o.schema 
      end
      return delta
    end

    alias <= merge

    def pending_merge(o)
      delta = o.map {|i| self.pending_insert(i)}
      if self.schema.empty? and o.respond_to?(:schema) and not o.schema.empty? then 
        self.schema = o.schema 
      end
      return delta
    end

    superator "<+" do |o|
      pending_merge o
    end

    def [](key)
      (@storage.include? key) ? tuple_accessors(key + @storage[key]) : nil
    end

    def method_missing(sym, *args, &block)
      @storage.send sym, *args, &block
    end

    alias reduce inject
  end

  class BloomScratch < BloomCollection
  end
  
  class BloomChannel < BloomCollection
    attr_accessor :locspec

    def split_locspec(l)
      lsplit = l.split(':')
      lsplit[1] = lsplit[1].to_i
      return lsplit
    end
    
    def establish_connection(l)
      $connections[l] = EventMachine::connect l[0], l[1], Server
      # rescue
      #   puts "connection #{l} failed"
    end
    
    def flush
      ip = Bloom::instance_variable_get('@ip')
      port = Bloom::instance_variable_get('@port')
      each_pending do |t|
        locspec = split_locspec(t[@locspec])
        # remote channel tuples are sent and removed
        if locspec != [ip, port] then
          establish_connection(locspec) if $connections[locspec].nil?
          $connections[locspec].send_data [@name, t].to_msgpack
          @pending.delete t
        end
      end
    end
  end
  
  class BloomPeriodic < BloomCollection
  end
  
  class BloomTable < BloomCollection
    def initialize(name, keys, cols)
      super(name, keys,cols)
      @to_delete = {}
    end

    def clone
      retval = super
      retval.to_delete = @to_delete.clone
    end

    def tick
      @to_delete.each_key {|t| @storage.delete t}
      @storage.merge! @pending
      @to_delete = {}
      @pending = {}
    end

    superator "<-" do |o|
      # delta = 
      o.map {|i| self.do_insert(i, @to_delete)}
    end
  end

  class BloomJoin < BloomCollection  
    attr_accessor :rels

    def initialize(*rels)
      @rels = rels
      @schema = []
      
      # turn into a tree of binary BloomJoins
      rels[1] = BloomJoin.new(*@rels[1..@rels.length-1]) unless rels.length == 2
      
      # now derive schema: combo of rels[0] and rels[1]
      if rels[0].schema.empty? or rels[1].schema.empty? then
        @schema = []
      else
        dups = rels[0].schema & rels[1].schema
        bothschema = rels[0].schema + rels[1].schema
        @schema = bothschema.to_enum(:each_with_index).map  {|c,i| if dups.include?(c) then c + '_' + i.to_s else c end }
      end
    end

    def do_insert(o,store)
      raise BloomError, "no insertion into joins"
    end

    def each
      @rels[0].each do |r|
        @rels[1].each do |s|
          yield [r] + [s]
        end  
      end
    end
  end

  ######## error types  
  class BloomError < Exception
  end

  class KeyConstraintError < BloomError
  end


  ######## the EventMachine server for handling network and timers
  class Server < EM::Connection
    attr_accessor :bloom

    def initialize(*args)
      @pac = MessagePack::Unpacker.new
      super
    rescue Exception
      print "An error occurred initializing BloomServer: ",$!, "\n"
    end

    def post_init
      @port, @ip = Socket.unpack_sockaddr_in(get_peername)
      puts "-- server inbound connection from #{@ip}:#{@port}"
      $connections = {} if $connections.nil?
      $connections[[@ip, @port]] = self
    rescue Exception
      print "An error occurred post_init on BloomServer: ",$!, "\n"
    end

    def receive_data(data)
      # Feed the received data to the deserializer
      @pac.feed data

      # streaming deserialize
      @pac.each do |obj|
        message_received(obj)
      end
    end

    def message_received(obj)
      puts "got " + obj.inspect
      if (obj.class <= Array and obj.length == 2 and not bloom.tables[obj[0].to_sym].nil? and obj[1].class <= Array) then
        $inbound << obj
        bloom.tick
      else
        puts " ... BAD!"
        bloom.tick
      end
    end

    def unbind
      puts "-- connection ended from #{@ip}:#{@port}"
      $connections.delete [@ip,@port]
    end
  end
end
alias rules lambda

