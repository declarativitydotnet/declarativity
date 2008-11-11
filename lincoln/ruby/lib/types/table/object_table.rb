require "lib/types/table/basic_table.rb"

class ObjectTable < BasicTable
  def initialize(context, name, key, types) # TableName, Key, TypeList
    super(context, name, key, types)
  end
end
