
        def get_scope(pred,program)
                scopeName = pred.to_s.split("::")
                scope = program
                tname = pred
                unless (scopeName[1].nil?) then
                        scope = scopeName[0]
                        tname = scopeName[1]
                end
                return [scope,tname]
        end

# had to.  will revisit.

       def ctab_lookup(name)
                tn = TableName.new(CompilerCatalogTable::COMPILERSCOPE,name)
                ret = Table.find_table(tn)
                raise("parser table #{name} not found in catalog") if ret.nil?
                return ret
        end

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
  raise "non-existent otab!" if otab.nil?
	tup = Tuple.new(*set)
	tup.schema = otab.schema_of

	#print "TUPLE: "+tup.inspect+"\n"
	ts = TupleSet.new(otab.name.to_s,tup)
	otab.insert(ts,nil)
end


# carpal tunnel
def predoftable(table)
        schema = table.schema_of
	print "SCHEMA IS #{schema}\n"
        return Predicate.new(false,table.name,table,schema.variables)

end

def joinwith(pred,table)
        sj = ScanJoin.new(pred,table.schema_of)
        ts = TupleSet.new("terms",*table.tuples)
        return sj.evaluate(ts)
end


def lcl_pretty_print(tuple)
        (0..tuple.size-1).each do |i|
                print "\t["+i.to_s+"] "+tuple.schema.variables[i].name+" = "+tuple.values[i].to_s+"\n"
        end
	print "--------------------\n"
end

