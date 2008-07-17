class Arguments 
  include Comparable
  include Enumerable

  attr_reader :predicate

  def initialize(predicate,arguments) 
    @args = Array.new
    @predicate = predicate
    arguments.each{|a| @args << a}
  end

  def to_s
    return @args.to_s
  end

  def <=>(a) 
    return @predicate.<=>(a.predicate);
  end
  
  def length
    return @args.length
  end
  
  def each
    @args.each do |a|
      yield a
    end
  end

  def [](i)
    return @args[i]
  end
end
