require "lib/types/table/callback"
class Listener < Callback
  def deletion(tups)
    tups.map {|t| @table.delete(t)}
  end

  def insertion(tups)
    tups.map {|t| @table.insert(t)}
  end
end # Index.Listener
