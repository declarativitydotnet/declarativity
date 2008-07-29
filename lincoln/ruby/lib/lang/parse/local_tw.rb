# TreeWalker callbacks specific to parsing Overlog programs

require "rubygems"
require "treetop"
require "lib/lang/parse/core.rb"
#require "olg.rb"

require "lib/lang/parse/procedural.rb"
require 'lib/types/basic/tuple'
require "lib/lang/parse/tree_walker.rb"

# um... symbol tables?
@@state = Hash.new
@@positions = Hash.new
@@current = Hash.new
@@lines = 0
@@verbose = 0 

class OverlogCompiler



# local modules

class VisitGeneric < TreeWalker::Handler

	def print_table(name,tuple)
		if (@@verbose.eql?("v")) then
			print name+"("+tuple.join(",")+");\n"
		end
	end

	def initialize()
		@@positions["_Universal"] = @@positions["_Termpos"] = @@positions["_Exprpos"] = @@positions["_Primpos"] = @@positions["Rule"] = 0
	end 
	def semantic(text,obj)
		@@positions["_Universal"] = @@positions["_Universal"] + 1
		if (@@positions[self.token].nil?) then
			@@positions[self.token] = 0
		else
			@@positions[self.token] = @@positions[self.token] + 1
		end
		@@state[self.token] = [text,@@positions[self.token]]
	end
end

class VisitBase < VisitGeneric
	def semantic(text,obj)
		super(text,obj)
		@@positions["_Termpos"] = @@positions["_Primpos"] = @@positions["_Exprpos"] = -1
	end
end

class VisitIExpression < VisitGeneric
	def initialize(expt)
		@ext = expt
		super()
	end
	def semantic(text,obj)
		super(text,obj)
		@@positions["_Exprpos"] = @@positions["_Exprpos"] + 1
		@@positions["_Primpos"] = -1
		print_table("expression",[@@positions["_Universal"],@@current["term"],@@positions["_Exprpos"],text,"expr","??"])
		#@ext.insert(TupleSet.new("expression",Tuple.new(@@positions["_Universal"],@@current["term"],@@positions["_Exprpos"],'"'+text+'"',"expr","??")),nil)
		otabinsert(@ext,@@positions["_Universal"],@@current["term"],@@positions["_Exprpos"],text)
		@@current["expression"] =  @@positions["_Universal"]
	end
end

class VisitTerm < VisitGeneric
	def initialize(pt)
		super()
		@termt = pt
	end
	def semantic(text,obj)
		super(text,obj)
		@@positions["_Primpos"] = @@positions["_Exprpos"] = -1
		@@positions["_Termpos"] = @@positions["_Termpos"] + 1
		@@current["term"] = @@positions["_Universal"]
		print "\n"
		print_table("term",[@@positions["_Universal"],@@positions["Rule"],@@positions["_Termpos"],text])
		#@termt.insert(TupleSet.new("term",Tuple.new(@@positions["_Universal"],@@positions["Rule"],@@positions["_Termpos"],text)),nil)
		otabinsert(@termt,@@positions["_Universal"],@@positions["Rule"],@@positions["_Termpos"],text)
		@@positions["_Universal"] = @@positions["_Universal"] + 1
	end
end

class VisitPexp < VisitGeneric
	def semantic(text,obj)
		super(text,obj)
		@@positions["_Primpos"] = @@positions["_Primpos"] + 1
	end
end


# "real" subclasses

class VisitProgram < VisitBase
	def initialize(pt)
		@prt = pt
	end
	def semantic(text,obj)
		super(text,obj)
		otabinsert(@prt,@@positions["_Universal"],nil,text)
	end
end

class VisitPredicate < VisitTerm
	def initialize (pt,term)
		super(term)
		@pt = pt
	end
	def semantic(text,obj)
		super(text,obj)
		print_table("predicate",[@@positions["_Universal"],@@current["term"],'"'+@@state["Predicate"][0]+'"',@@positions["_Termpos"],nil])
		#result = @pt.insert(TupleSet.new("predicate",Tuple.new(@@positions["_Universal"],@@current["term"],@@state["Predicate"][0],@@positions["_Termpos"])),nil)
		otabinsert(@pt,@@positions["_Universal"],@@current["term"],@@state["Predicate"][0],@@positions["_Termpos"])
		#print "PRED ARGS: "+obj.args.to_s+"\n"

	end
end

class VisitConstant < VisitPexp
	def initialize (pt)
		super()
		@pet = pt
	end
	def semantic(text,obj)
		super(text,obj)
		t = text.gsub('"',"")
		print_table("primaryExpression",[@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+t+'"',"const","??"])

		#@pet.insert(TupleSet.new("variable",Tuple.new(@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+t+'"',"const","??")),nil)
		otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+t+'"',"const","??")
	end
end

class VisitVariable < VisitPexp
	def initialize (pt)
		super()
		@pet = pt
	end

	def semantic(text,obj)
		super(text,obj)
		print_table("primaryExpression",[@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],text,"var","??"])
		#@pet.insert(TupleSet.new("variable",Tuple.new(@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+text+'"',"var","??")),nil)
		otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+text+'"',"var","??")
	end
end

class VisitFact < VisitTerm
	def initialize (facts,terms)
		@ft = facts
		super(terms)
	end
	def semantic(text,obj)
		@@positions["_Termpos"] = @@positions["_Primpos"] = -1
		super(text,obj)
		print_table("fact",[@@positions["_Universal"],@@current["term"],'"'+text+'"'])
		#@ft.insert(TupleSet.new("fact",Tuple.new(@@positions["_Universal"],@@current["term"],text)),nil)
		otabinsert(@ft,@@positions["_Universal"],@@current["term"],text)
	end
end

class VisitAggregate < VisitGeneric

	def initialize (pt)
		super()
		@pet = pt
	end
	def semantic(text,obj)
		super(text,obj)
		t = text.gsub('"',"")
		print_table("primaryExpression",[@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+t+'"',"agg_func","??"])

		#@pet.insert(TupleSet.new("function",Tuple.new(@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+obj.func.text_value+'"',"agg_func","??")),nil)
		otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+obj.func.text_value+'"',"agg_func","??")
	end
end

class VisitRule < VisitBase
	def semantic(text,obj)
		t = text.gsub('"',"")
		super(text,obj)

		#puts obj.elements.inspect
		if (defined? obj.elements.deleter) then
			print "OBJ DELETE!!!! ("+ obj.elements.deleter.to_s+")\n"
		end
		print_table("rule",[@@positions["_Universal"],@@positions["Program"],'"'+t+'"',-1,nil,nil])
	end
end

class VisitTable < VisitBase
	def initialize(pt)
		@tt = pt
	end
	def semantic(text,obj)
		super(text,obj)
		print_table("table",[@@positions["_Universal"],text])
		@@current["table"] = @@positions["_Universal"]
		#@tt.insert(TupleSet.new("table",Tuple.new(@@positions["_Universal"],text)),nil)
		otabinsert(@tt,@@positions["_Universal"],text)
	end
end

class VisitColumn < VisitGeneric
	def initialize(col)
		@col = col
	end	
	def semantic(text,obj)
		super(text,obj)	
		#coltab(@@positions["TableName"],@@positions["Type"],text);
		print_table("columns",[@@positions["TableName"],@@positions["Type"],'"'+text+'"'])
		#@col.insert(TupleSet.new("column",Tuple.new(@@current["table"],@@positions["_Universal"],text)),nil)
		otabinsert(@col,@@current["table"],@@positions["_Universal"],text)
	end
end

class VisitIndex < VisitGeneric
	def initialize(ix)
		@ix = ix
	end
	def semantic(text,obj)
		suck_nums(obj).each do |indx|
			print "\tINDEX: "+indx+" at "+@@current["table"].to_s+"\n"
			#indxtab(@@positions["TableName"],indx)
			print_table("index",[@@positions["TableName"],indx])

			puts @ix.to_s
			super(text,obj)
			#@ix.insert(TupleSet.new("index",Tuple.new(@@positions["_Universal"],@@current["table"],indx)),nil)
			otabinsert(@ix,@@positions["_Universal"],@@current["table"],indx)
		end
	end
end



class VisitAssignment < VisitTerm
	def initialize(ass,term)
		@pt = ass
		super(term)
	end
	def semantic(text,obj)
		super(text,obj)
		t = obj.Variable.text_value.gsub('"','\"')
		print_table("assign",[@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],text])
	end
end


class VisitSelection < VisitTerm
	def initialize(sel,term)
		@pt = sel
		super(term)
	end
	def semantic(text,obj)
		super(text,obj)
		print_table("select",[@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],text])
		t = text.gsub('"','\"')
	end
end


class VisitExpression < VisitIExpression
	def semantic(text,obj)
		#if (!defined? obj.PrimaryExpression) then
			super(text,obj)
			#print_table("expression",[@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],text,"expr","??"])
		#end
	end
end


class VisitNewline < VisitGeneric
	def semantic(text,obj)
		@@lines = @@lines + 1
	end
end


# class body


	def initialize(rules,terms,preds,pexps,exps,facts,tables,columns,indices,programs)

		@ruletable = rules
		@termtable = terms
		@predicatetable = preds
		@pextable = pexps
		@extable = exps
		@facttable = facts
		@tabletable = tables
		@columntable = columns
		@indextable = indices
		@programtable = programs


		# reinitialize these awful globals, fingers crossed for good garbage collection.
		@@state = Hash.new
		@@positions = Hash.new
		@@current = Hash.new
		@@lines = 0
	end
	def verbose=(v)
		@@verbose = v
	end	

	def parse(prog)
		parser = OverlogParser.new
		@tree = parser.parse(prog)
		if !@tree
			puts 'failure'
			raise RuntimeError.new(parser.failure_reason)
		end 
	end

	def analyze

		sky = TreeWalker.new(@tree)

		vg = VisitGeneric.new

		sky.add_handler("pprogramname",VisitProgram.new(@programtable),1)
		sky.add_handler("Word",vg,1)
		sky.add_handler("Location",vg,1)
		sky.add_handler("Watch",vg,1)
		sky.add_handler("expression",VisitExpression.new(@extable),1)
		sky.add_handler("Primaryexpression",vg,1)
		sky.add_handler("Predicate",VisitPredicate.new(@predicatetable,@termtable),1)
		sky.add_handler("Fact",VisitFact.new(@facttable,@termtable),1)
		sky.add_handler("Definition",VisitTable.new(@tabletable),1)
		sky.add_handler("TableName",vg,1)
		sky.add_handler("Schema",vg,1)
		sky.add_handler("Rule",VisitRule.new,1)
		sky.add_handler("Selection",VisitSelection.new(nil,@termtable),1)
		sky.add_handler("Assignment",VisitAssignment.new(nil,@termtable), 1)
		
		sky.add_handler("Variable",VisitVariable.new(@pextable),1)
		sky.add_handler("Constant",VisitConstant.new(@pextable),1)


		sky.add_handler("Aggregate",VisitAggregate.new(@pextable),1)
		sky.add_handler("Name",vg,1)
		sky.add_handler("AggregateVariable",VisitVariable.new(@pextable),1)

		#sky.add_handler("Arguments",vg,1)

		sky.add_handler("Periodic",vg,1)
		sky.add_handler("Table",VisitTable.new(@tabletable),1)
		sky.add_handler("Type",VisitColumn.new(@columntable),1)
		sky.add_handler("Keys",VisitIndex.new(@indextable),1)

		sky.add_handler("LineTerminator",VisitNewline.new,1)

		sky.add_handler("Program",vg,1)

		#init_output("static_checks")
		sky.walk("n")
		
	end
end
