require "lib/types/table/hash_index"
require "lib/types/exception/bad_key_exception"

class RefTable < Table
  def initialize(name, key, types)
    super(name, Type::TABLE, INFINITY, INFINITY, key, types)
    @key = key
    @tuples = TupleSet.new(name, nil)
    @primary = HashIndex.new(self, key, Index::Type::PRIMARY)
    @secondary = Hash.new
  end

  def tuples
    return (@tuples.nil? ? TupleSet.new(name) : @tuples)
  end

  def primary
    @primary
  end

  def secondary
    @secondary
  end

  def cardinality
    @tuples.size
  end

  protected  
    def delete_tup(t)
      if @tuples.include?(t) then
        lookup = primary.lookup(t)
      end

      lookup.each do |l|
        if l == t then
          if l.count > 1.0 then
            l.count = l.count - 1.0
            return false
          end
          return tuples.delete(t)
        end
      end unless lookup.nil?
      return false
    end

    def insert_tup(t)
      if @tuples.include?(t) then
        primary.lookup(t).each do |l|
          if l == t then
            l.count = l.count + 1.0
            return false
          end
        end
      end
      @tuples << t.clone
    end

end