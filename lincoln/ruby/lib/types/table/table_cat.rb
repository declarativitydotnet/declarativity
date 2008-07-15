require 'lib/types/table/table'
require 'lib/types/table/catalog'
require 'lib/types/table/index'

class TableCat
  @table = nil
  @catalog = nil
  @index = nil

  def table
    return @table
  end

  def catalog
    return @catalog
  end
  
  def index
    return @index
  end

  def init
    @table = Table.new
    @catalog = Catalog.new
    @index = Index::IndexTable.new
    register(catalog.name, catalog.type, catalog.size, catalog.lifetime.to_f, catalog.key, TypeList.new(Table.types), catalog)
  end
  
  def table(name)
    begin
      table = catalog.primary.lookup(name)
      if table.nil? then
        return nil
      elsif table.size == 1 then
        return table.next.value(Catalog.Field::OBJECT)
      elsif table.size > 1 then
        raise "More than one " + name + " table defined!"
        Kernel.exit(1)
      end
    rescue BadKeyException => boom
      boom.backtrace.join("\n")
    end
    return nil
  end
end