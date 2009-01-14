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
    def initialize(runtime)
      @@positions["_Universal"] = @@positions["_Termpos"] = @@positions["_Exprpos"] = @@positions["_Primpos"] = @@positions["Rule"] = @@positions["Fact"] = @@positions["Clause"] = @@positions["_Predarg"] = 0
      super(runtime)
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
    def initialize(runtime, expt)
      @ext = expt
      super(runtime)
    end
    def semantic(text,obj)
      @@positions["_Exprpos"] = @@positions["_Exprpos"] + 1
      @@positions["_Primpos"] = -1
      otabinsert(@ext,@@positions["_Universal"],@@current["term"],@@current["clause"],@@positions["_Predarg"],@@positions["_Exprpos"],text)
      #otabinsert(@ext,@@positions["_Universal"],@@current["term"],@@positions["_Exprpos"],text)

      @@current["expression"] =  @@positions["_Universal"]
      super(text,obj)
    end
  end

  class VisitTerm < VisitGeneric
    def initialize(runtime, pt)
      super(runtime)
      @termt = pt
    end
    def semantic(text,obj)
      super(text,obj)
      @@positions["_Primpos"] = @@positions["_Predarg"] =  @@positions["_Exprpos"] = -1
      @@positions["_Termpos"] = @@positions["_Termpos"] + 1
      @@current["term"] = @@positions["_Universal"]
      otabinsert(@termt,@@positions["_Universal"],@@current["clause"],@@positions["_Termpos"],text)
      @@current["term"] = @@positions["_Universal"]
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
    def initialize(runtime, pt)
      super(runtime)
      @prt = pt
    end
    def semantic(text,obj)
      super(text,obj)
      otabinsert(@prt,@@positions["_Universal"],nil,text)
      @@current["program"] = @@positions["_Universal"]

    end
  end

  class VisitPredicate < VisitTerm
    def initialize (runtime, pt,term)
      super(runtime, term)
      @pt = pt
    end
    def semantic(text,obj)
      super(text,obj)
      #     # require 'ruby-debug'; debugger if obj.ptablename.text_value == 'execute'
      eventMod = obj.eventModifier.text_value.eql?("") ? nil : obj.eventModifier.elements[1].text_value
      notin = obj.notin.text_value.eql?("") ? false : true
      @@current["predicate"] = @@positions["_Universal"]
      otabinsert(@pt,@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],obj.ptablename.text_value,eventMod,notin)
    end
  end

  class VisitTableFunction < VisitPredicate
    def initialize(runtime, tft,pred,term)
      super(runtime, pred,term)
      @tft = tft
    end
    def semantic(text,obj)
      super(text,obj.notapredicate)
      @@positions["_Universal"] = @@positions["_Universal"] + 1;
      otabinsert(@tft, @@positions["_Universal"], @@current["term"], obj.ptablename.text_value, @@current["predicate"])    
    end
  end

  class VisitPredArg < VisitGeneric
    def semantic(text,obj)
      super(text,obj)
      #        # require 'ruby-debug'; debugger
      @@positions["_Predarg"] = @@positions["_Predarg"] + 1
    end
  end

  class VisitConstant < VisitPexp
    def initialize (runtime, pt,et)
      super(runtime, et)
      @pet = pt
    end
    def semantic(text,obj)
      t = text.gsub('"',"")
      super(t,obj)
      otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],t,"const","??")
    end
  end

  class VisitVariable < VisitPexp
    def initialize (runtime, pt,et)
      super(runtime, et)
      @pet = pt
    end

    def semantic(text,obj)
      super(text,obj)
      otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],text,"var","??")
    end
  end

  class VisitWatch < VisitTerm
    def initialize (r, tabs,terms)
      @tabs =	tabs 
      super(r, terms)
    end
    def semantic(text,obj)
#      # require 'ruby-debug'; debugger
      @@positions["_Termpos"] = -1
      super(text,obj)
      # the table must already exist.  Recreating this index is costly.  replace soon.
      tabtab = @runtime.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myTable"))
      if tabtab.nil?
        #        # require 'ruby-debug'; debugger
        raise("no tabletable, requesting watch on #{obj.ptablename.text_value}") 
      end
      #     # require 'ruby-debug'; debugger
      hi = HashIndex.new(@runtime,tabtab,Key.new(1),String)
      tab = hi.lookup(Tuple.new(nil,obj.ptablename.text_value))
      if tab.tups[0].nil?
        #        # require 'ruby-debug'; debugger
        raise("no tuples in tabletable, requesting watch on #{obj.ptablename.text_value}")
      end
      ptr = tab.tups[0].clone
      tabtab.delete(tab.tups)

      otabinsert(tabtab,ptr.name_value("tableid"),ptr.name_value("tablename"),obj.watchword.watchFlow.text_value)


    end
  end



  class VisitAggregate < VisitVariable

    def initialize (r, pt,et)
      super(r,pt,et)
      @pet = pt
    end
    def semantic(text,obj)
      #super(text,obj)
      t = text.gsub('"',"")
      #      # require 'ruby-debug'; debugger
      otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],obj.func.text_value,"agg_func","??")

      super(obj.aggregatevariable.text_value,obj.aggregatevariable)

      #@@positions["_Universal"] = @@positions["_Universal"] + 1
      #@@positions["_Primpos"] = @@positions["_Primpos"] + 1
      #otabinsert(@pet,@@positions["_Universal"],@@current["expression"],@@positions["_Primpos"],'"'+obj.aggregatevariable.text_value+'"',"var","??")
    end
  end

  class VisitClause < VisitBase
    def initialize(r,ct)
      super r
      @ct = ct
    end
    def semantic(text,obj,type)
      #     # require 'ruby-debug'; debugger
      super(text,obj)
      # table MyClause (
      #   +clauseid Integer,
      #   clause_type String)
      ruleid = type==Rule ? @@current["rule"] : nil
      factid = type==Fact ? @@current["fact"] : nil
      otabinsert(@ct,@@positions["_Universal"],type)
      @@current["clause"] = @@positions["_Universal"]
    end
  end

  class VisitFact < VisitClause
    def initialize (runtime, facts, clauses)
      @ft = facts
      super(runtime, clauses)
    end
    def semantic(text,obj)
      require 'ruby-debug'; debugger
      #@@positions["_Termpos"] = @@positions["_Primpos"] = -1
      #      @@positions["_Termpos"] = -1
      super(text,obj,Fact)
      # table MyFact (
      #   +factid Integer,
      #   programid Integer,
      #   clauseid Integer,
      #   tablename String,
      #   term_text String
      # )
      otabinsert(@ft,@@positions["_Universal"],@@current["program"],@@current["clause"],obj.ptablename.text_value,text)
      @@current["fact"] = @@positions["_Universal"]
    end
  end

  class VisitRule < VisitClause
    def initialize (r,pt,ct)
      super(r,ct)
      @rt = pt
    end
    def semantic(text,obj)
      t = text.gsub('"',"")
      super(text,obj,Rule)

      if obj.deleter.delete then
        d = 1
      else
        d = 0
      end
      # rule name.
      if (defined? obj.rname) then
        name = obj.rname.text_value
      else 
        name = text[0..10].gsub(/\n/,' ') + "..."
      end


      # table MyRule (
      #   +ruleid Integer,
      #   programid Integer,
      #   clauseid Integer,
      #   rulename String,
      #   public Integer,
      #   delete Integer
      # )
      otabinsert(@rt,@@positions["_Universal"],@@current["program"],@@current["clause"],name,nil,d)
      @@current["rule"] = @@positions["_Universal"]
    end
  end

  class VisitTable < VisitBase
    def initialize(r,pt)
      @tt = pt
      super(r)
    end
    def semantic(text,obj)
      super(text,obj)
      @@current["table"] = @@positions["_Universal"]
      #@tt.insert(TupleSet.new("table",Tuple.new(@@positions["_Universal"],text)),nil)
      ## require 'ruby-debug'; debugger
      otabinsert(@tt,@@positions["_Universal"],text,nil)
    end
  end

  class VisitColumn < VisitGeneric
    def initialize(r,col)
      @col = col
      super(r)
    end	
    def semantic(text,obj)
      super(text,obj)	
      @@positions["_Primpos"] = @@positions["_Primpos"] + 1
      otabinsert(@col,@@positions["_Universal"],@@current["table"],@@positions["_Primpos"],text)
    end
  end

  class VisitSQLIndex < VisitGeneric

    def initialize(r,ix)
      @ix = ix
      super(r)
    end
    def semantic(text,obj)
      otabinsert(@ix,@@positions["_Universal"],@@current["table"],@@positions["_Primpos"])
    end

  end

  class VisitIndex < VisitGeneric
    def initialize(r,ix)
      @ix = ix
      super(r)
    end
    def semantic(text,obj)
      if suck_nums(obj).size == 0 then
        # make sure we set up a nil key
        otabinsert(@ix,@@positions["_Universal"],@@current["table"],nil)
      else
        suck_nums(obj).each do |indx|
          super(text,obj)
          otabinsert(@ix,@@positions["_Universal"],@@current["table"],indx)
        end
      end
    end
  end



  class VisitAssignment < VisitTerm
    def initialize(r,ass,term)
      @at = ass
      super(r,term)
    end
    def semantic(text,obj)
      super(text,obj)
      t = obj.variable.text_value.gsub('"','\"')
      otabinsert(@at,@@positions["_Universal"],@@current["term"],@@positions["_Termpos"],obj.variable.text_value,obj.expression.text_value)
    end
  end


  class VisitSelection < VisitTerm
    def initialize(r,sel,term)
      @st = sel
      super(r,term)
    end
    def semantic(text,obj)
      super(text,obj)
      t = text.gsub('"','\"')
      otabinsert(@st,@@positions["_Universal"], @@current["term"], @@positions["_Termpos"],text)
    end
  end


  class VisitExpression < VisitIExpression
    def semantic(text,obj)
      #      # require 'ruby-debug'; debugger if text =~ /TableName/
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


  def initialize(runtime, rules,terms,preds,pexps,exps,facts,tables,columns,indices,programs,assigns,selects,tfuncs,clauses,mcalls)
    @runtime = runtime
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
    @tablefunctiontable = tfuncs
    @clausetable = clauses

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

    vg = VisitGeneric.new(@runtime)

    sky.add_handler("Word",vg,1)
    sky.add_handler("Location",vg,1)
    sky.add_handler("Watch",VisitWatch.new(@runtime,@tabletable,@termtable),1)
    sky.add_handler("Require", VisitRequire.new(@runtime),1)
    sky.add_handler("expression",VisitExpression.new(@runtime,@extable),1)
    sky.add_handler("primaryexpression",vg,1)
    sky.add_handler("predicate",VisitPredicate.new(@runtime,@predicatetable,@termtable),1)
    sky.add_handler("TableFunction",VisitTableFunction.new(@runtime,@tablefunctiontable,@predicatetable,@termtable),1)
    sky.add_handler("Fact",VisitFact.new(@runtime,@facttable,@clausetable),1)
    sky.add_handler("Definition",VisitTable.new(@runtime,@tabletable),1)
    sky.add_handler("TableName",vg,1)
    sky.add_handler("Schema",vg,1)
    sky.add_handler("Rule",VisitRule.new(@runtime,@ruletable,@clausetable),1)
    sky.add_handler("Selection",VisitSelection.new(@runtime,@selecttable,@termtable),1)
    sky.add_handler("Assignment",VisitAssignment.new(@runtime,@assigntable,@termtable), 1)

    sky.add_handler("variable",VisitVariable.new(@runtime,@pextable,@extable),1)
    sky.add_handler("Constant",VisitConstant.new(@runtime,@pextable,@extable),1)

    #sky.add_handler("name",vg,1)


    sky.add_handler("Aggregate",VisitAggregate.new(@runtime,@pextable,@extable),1)
    sky.add_handler("Name",vg,1)
    #sky.add_handler("AggregateVariable",VisitVariable.new(@runtime,@pextable,@extable),1)

    #sky.add_handler("Arguments",vg,1)

    sky.add_handler("Periodic",vg,1)
    sky.add_handler("Table",VisitTable.new(@runtime,@tabletable),1)
    sky.add_handler("Type",VisitColumn.new(@runtime,@columntable),1)
    sky.add_handler("Keys",VisitIndex.new(@runtime,@indextable),1)

    sky.add_handler("tablename",VisitTable.new(@runtime,@tabletable),1)
    sky.add_handler("dtype",VisitColumn.new(@runtime,@columntable),1)
    sky.add_handler("key_modifier",VisitSQLIndex.new(@runtime,@indextable),1)

    sky.add_handler("LineTerminator",VisitNewline.new(@runtime),1)

    sky.add_handler("pprogramname",VisitProgram.new(@runtime,@programtable),1)

    sky.add_handler("uExpression",VisitPredArg.new(@runtime),1)

    #init_output("static_checks")
    sky.walk("n")

  end
end
