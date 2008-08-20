# TreeWalker callbacks specific to parsing Overlog programs

require "rubygems"
require "treetop"

require "lib/lang/parse/ddl.rb"
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
	def initialize()
		@@positions["_Universal"] = @@positions["_Termpos"] = @@positions["_Exprpos"] = @@positions["_Primpos"] = @@positions["Rule"] = @@positions["_Predarg"] = 0
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
		@@positions["_Exprpos"] = @@positions["_Exprpos"] + 1
		@@positions["_Primpos"] = -1
		otabinsert(@ext,@@positions["_Universal"],@@current["term"],@@positions["_Predarg"],@@positions["_Exprpos"],text)
		#otabinsert(@ext,@@positions["_Universal"],@@current["term"],@@positions["_Exprpos"],text)

		@@current["expression"] =  @@positions["_Universal"]
		super(text,obj)
	end
end

class VisitTerm < VisitGeneric
	def initialize(pt)
		super()
		@termt = pt
	end
	def semantic(text,obj)
		super(text,obj)
		@@positions["_Primpos"] = @@positions["_Predarg"] =  @@positions["_Exprpos"] = -1
		@@positions["_Termpos"] = @@positions["_Termpos"] + 1
		@@current["term"] = @@positions["_Universal"]
		otabinsert(@termt,@@positions["_Universal"],@@current["rule"],@@positions["_Termpos"],text)
		@@positions["_Universal"] = @@positions["_Universal"] + 1
	end
end

class VisitPexp < VisitIExpression
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
		@@current["program"] = @@positions["_Universal"]

	end
end

class VisitPredicate < VisitTerm
	def initialize (pt,term)
		super(term)
		@pt = pt
	end
	def semantic(text,obj)
		super(text,obj)

		#puts obj.inspect
		eventMod = obj.eventModifier.text_value.eql?("") ? nil : obj.eventModifier.elements[1].text_value
		#otabinsert(@pt,@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],@@state["Predicate"][0])
		otabinsert(@pt,@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],obj.ptablename.text_value,eventMod)
	end
end

class VisitPredArg < VisitGeneric
	def semantic(text,obj)
		super(text,obj)
		@@positions["_Predarg"] = @@positions["_Predarg"] + 1
	end
end

class VisitConstant < VisitPexp
	def initialize (pt,et)
		super(et)
		@pet = pt
	end
	def semantic(text,obj)
		t = text.gsub('"',"")
		super(t,obj)
		otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],t,"const","??")
	end
end

class VisitVariable < VisitPexp
	def initialize (pt,et)
		super(et)
		@pet = pt
	end

	def semantic(text,obj)
		super(text,obj)
		otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],text,"var","??")
	end
end

class VisitFact < VisitTerm
	def initialize (facts,terms)
		@ft = facts
		super(terms)
	end
	def semantic(text,obj)
		#@@positions["_Termpos"] = @@positions["_Primpos"] = -1
		@@positions["_Termpos"] = -1
		super(text,obj)
		otabinsert(@ft,@@positions["_Universal"],@@current["term"],text)
	end
end

class VisitAggregate < VisitVariable

	def initialize (pt,et)
		super(pt,et)
		@pet = pt
	end
	def semantic(text,obj)
		#super(text,obj)
		t = text.gsub('"',"")
		otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],obj.func.text_value,"agg_func","??")

		super(obj.aggregatevariable.text_value,obj.aggregatevariable)

		#@@positions["_Universal"] = @@positions["_Universal"] + 1
		#@@positions["_Primpos"] = @@positions["_Primpos"] + 1
		#otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+obj.aggregatevariable.text_value+'"',"var","??")
	end
end

class VisitRule < VisitBase
	def initialize (pt)
		super()
		@rt = pt
	end
	def semantic(text,obj)
		t = text.gsub('"',"")
		super(text,obj)

		if obj.deleter.delete then
			d = 1
		else
			d = 0
		end
		# rule name.
		if (defined? obj.name) then
			name = obj.name.text_value
		else 
			name = text.hash.to_s
		end
		
		otabinsert(@rt,@@positions["_Universal"],@@current["program"],name,nil,d)
		@@current["rule"] = @@positions["_Universal"]
	end
end

class VisitTable < VisitBase
	def initialize(pt)
		@tt = pt
	end
	def semantic(text,obj)
		super(text,obj)
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
		@@positions["_Primpos"] = @@positions["_Primpos"] + 1
		otabinsert(@col,@@positions["_Universal"],@@current["table"],@@positions["_Primpos"],text)
	end
end

class VisitSQLIndex < VisitGeneric

	def initialize(ix)
		@ix = ix
	end
	def semantic(text,obj)
		otabinsert(@ix,@@positions["_Universal"],@@current["table"],@@positions["_Primpos"])
	end

end

class VisitIndex < VisitGeneric
	def initialize(ix)
		@ix = ix
	end
	def semantic(text,obj)
		suck_nums(obj).each do |indx|
			super(text,obj)
			otabinsert(@ix,@@positions["_Universal"],@@current["table"],indx)
		end
	end
end



class VisitAssignment < VisitTerm
	def initialize(ass,term)
		@at = ass
		super(term)
	end
	def semantic(text,obj)
		super(text,obj)
		t = obj.variable.text_value.gsub('"','\"')
		otabinsert(@at,@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],obj.variable.text_value,obj.expression.text_value)
	end
end


class VisitSelection < VisitTerm
	def initialize(sel,term)
		@st = sel
		super(term)
	end
	def semantic(text,obj)
		super(text,obj)
		t = text.gsub('"','\"')
		otabinsert(@st,@@positions["_Universal"], @@current["term"], @@positions["_Termpos"],text)
	end
end


class VisitExpression < VisitIExpression
	def semantic(text,obj)
		if (!defined? obj.primaryexpression) then
			super(text,obj)
		end
	end
end


class VisitNewline < VisitGeneric
	def semantic(text,obj)
		@@lines = @@lines + 1
	end
end

class VisitRequire < VisitGeneric
  def semantic(text,obj)
    require text
    super(text,obj)
  end
end

# class body


	def initialize(rules,terms,preds,pexps,exps,facts,tables,columns,indices,programs,assigns,selects)

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
		@selecttable = selects
		@assigntable = assigns

		# reinitialize these awful globals, fingers crossed for good garbage collection.
		@@state = Hash.new
		@@positions = Hash.new
		@@current = Hash.new
		@@lines = 0
	end
	def verbose=(v)
		@@verbose = v
	end	

  attr_reader :parser, :tree

	def parse(prog)
		@parser = OverlogParser.new
		@tree = parser.parse(prog)
		if !@tree
			raise RuntimeError.new(parser.failure_reason)
		end 
	end
      

	def analyze

		sky = TreeWalker.new(@tree)

		vg = VisitGeneric.new

		sky.add_handler("Word",vg,1)
		sky.add_handler("Location",vg,1)
		sky.add_handler("Watch",vg,1)
		sky.add_handler("Require", VisitRequire.new,1)
		sky.add_handler("expression",VisitExpression.new(@extable),1)
		sky.add_handler("primaryexpression",vg,1)
		sky.add_handler("predicate",VisitPredicate.new(@predicatetable,@termtable),1)
		sky.add_handler("Fact",VisitFact.new(@facttable,@termtable),1)
		sky.add_handler("Definition",VisitTable.new(@tabletable),1)
		sky.add_handler("TableName",vg,1)
		#sky.add_handler("TableName",VisitTable.new(@tabletable),1)
		
		sky.add_handler("Schema",vg,1)
		sky.add_handler("Rule",VisitRule.new(@ruletable),1)
		sky.add_handler("Selection",VisitSelection.new(@selecttable,@termtable),1)
		sky.add_handler("Assignment",VisitAssignment.new(@assigntable,@termtable), 1)
		
		sky.add_handler("variable",VisitVariable.new(@pextable,@extable),1)
		sky.add_handler("Constant",VisitConstant.new(@pextable,@extable),1)


		sky.add_handler("Aggregate",VisitAggregate.new(@pextable,@extable),1)
		sky.add_handler("Name",vg,1)
		#sky.add_handler("AggregateVariable",VisitVariable.new(@pextable,@extable),1)

		#sky.add_handler("Arguments",vg,1)

		sky.add_handler("Periodic",vg,1)
		sky.add_handler("Table",VisitTable.new(@tabletable),1)
		sky.add_handler("Type",VisitColumn.new(@columntable),1)
		sky.add_handler("Keys",VisitIndex.new(@indextable),1)

		sky.add_handler("tablename",VisitTable.new(@tabletable),1)
		sky.add_handler("dtype",VisitColumn.new(@columntable),1)
		sky.add_handler("key_modifier",VisitSQLIndex.new(@indextable),1)

		sky.add_handler("LineTerminator",VisitNewline.new,1)

		sky.add_handler("pprogramname",VisitProgram.new(@programtable),1)

		sky.add_handler("uExpression",VisitPredArg.new,1)

		#init_output("static_checks")
		sky.walk("n")
		
	end
end
