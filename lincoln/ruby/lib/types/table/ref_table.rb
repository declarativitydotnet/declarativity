require "lib/types/table/hash_index"
require "lib/types/exception/bad_key_exception"
require "lib/types/table/table"

class RefTable < Table
  def initialize(context, name, key, types)
    super(name, TableType::TABLE, key, types)
    @key = key
    @tuples = TupleSet.new(name, nil)
    @primary = HashIndex.new(context, self, key, Index::Type::PRIMARY)
    @secondary = Hash.new
  end

  def tuples
    return (@tuples.nil? ? TupleSet.new(name) : @tuples)
  end
  
  attr_reader :primary, :secondary

  def cardinality
    @tuples.size
  end

  protected  
    def delete_tup(t)
      if @tuples.include?(t) then
        lookup = primary.lookup(t)
      end

      return nil if lookup.nil?
      lookup.each do |l|
        if l == t then
          if l.count > 1.0 then
            l.count = l.count - 1.0
            return nil
          end
          return tuples.delete(t)
        end
      end
      return nil
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
      return true
    end

end