require "lib/types/table/callback"
class Listener < Callback
  def deletion(tups)
    tups.each {|t| @table.delete(t)}
  end

  def insertion(tups)
    tups.each {|t| @table.insert(t)}
  end
end # Index.Listener
