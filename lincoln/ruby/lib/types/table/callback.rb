require 'lib/types/table/table'

class Callback
  @@ids = 0

  def initialize(tab)
    @table = tab
    @id = @@ids
    @@ids += 1
  end

  def insertion(tuples)
   raise "Callback with no insertion implementation"
  end

  def deletion(tuples)
   raise "Callback with no deletion implementation"
  end
end # Callback
