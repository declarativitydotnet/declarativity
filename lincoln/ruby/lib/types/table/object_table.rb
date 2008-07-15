require "lib/types/table/basic_table.rb"

class ObjectTable < BasicTable
  def initialize(name, key, types) # TableName, Key, TypeList
    super(name,INFINITY, INFINITY, key, types)
  end
  
end