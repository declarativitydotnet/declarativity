class Arguments 
  include Comparable
  include Enumerable

  def initialize(predicate,arguments) 
    @args = Array.new
    @predicate = predicate
    arguments.each{|a| @args << a}
  end

  def predicate
    @predicate
  end

  def to_s
    return @args.to_s
  end

  def <=>(a) 
    return @predicate.<=>(a.predicate);
  end
  
  def each
    @args.each do |a|
      yield a
    end
  end

end
