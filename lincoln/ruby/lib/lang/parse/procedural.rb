
# had to.  will revisit.
def suck_nums(node)
        returns  = Array.new
        if (defined? node.DecimalNumeral) then
                returns << node.DecimalNumeral.text_value
        else
                returns = Array.new
                if (!node.elements.nil?) then
                        node.elements.each do |e|
                                list = suck_nums(e)
                                if (!list.empty?) then
                                        returns.concat(list)
                                end
                        end
                end
        end
        return returns
end


# this one makes a little sense

def otabinsert(otab,*set)
	tup = Tuple.new(*set)

	tup.schema = otab.schema_of

	#print "TUPLE: "+tup.inspect+"\n"
	ts = TupleSet.new(otab.name.to_s,tup)
	otab.insert(ts,nil)
end


# carpal tunnel
def predoftable(table)
        schema = table.schema_of
        return Predicate.new(false,table.name,table,schema.variables)

end

def joinwith(pred,table)
        sj = ScanJoin.new(pred,table.schema_of)
        ts = TupleSet.new("terms",*table.tuples)
        return sj.evaluate(ts)
end

