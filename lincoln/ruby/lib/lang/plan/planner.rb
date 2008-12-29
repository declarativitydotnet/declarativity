require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'
require 'lib/lang/parse/schema.rb'
require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/key_schema.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"
require "lib/lang/plan/value.rb"
require "lib/lang/plan/arbitrary_expression.rb"
require "lib/types/function/aggregate_fn.rb"
require "lib/lang/plan/watch_clause.rb"
require "lib/lang/parse/procedural.rb"
require 'lib/core/driver'
require 'lib/lang/parse/schema.rb'
require 'benchmark'

class OverlogPlanner

  class PlanProjector
    def initialize(runtime, programname,ts)
      @runtime = runtime
      @tuples = ts
      @progname = programname
    end

    def proj_cat(tup,obj,type,func,position)
      proj = func.call(tup)
      #			# require 'ruby-debug'; debugger if not defined? obj.position
      objVar = Variable.new(type,obj.class,position,nil)
      proj.append(objVar,obj)
      return proj
    end

    def emit
      # the base class just returns the tuples it has accumulated
      # if we order correctly now, we should never need to order again, except when
      # ordering is lost by stuff like hash tables.
      return @tuples
    end
  end

  class PlanExpression < PlanProjector	
    class VarEater
      # I know, too many nested classes
      def initialize
        @aggFunc = nil	
      end
      def eat(var)
        # this may be called on tuples that have no Vars in them,
        # e.g. TableFunction tuples.
        # return nil for those -- JMH
        return nil if var.schema.position("type").nil?

        if (!@aggFunc.nil?) then
          if (!var.name_value("type").eql?("var")) then
            #					  # require 'ruby-debug'; debugger
            raise("can't aggregate over non-variable ("+var.name_value("type")+")")
          end
          # fix that string stuff!
          #          # require 'ruby-debug'; debugger
          thisvar = Aggregate.new(var.name_value("p_txt"),@aggFunc,AggregateFunction.agg_type(@aggFunc,String), var.name_value("expr_pos")) #, nil)
          thisvar.position = var.name_value("expr_pos")
          @aggFunc = nil
        else 
          case var.name_value("type")
          when "var"
            thisvar = Variable.new(var.name_value("p_txt"),String,var.name_value("expr_pos"),nil)
          when "const"
            thisvar = Value.new(var.name_value("p_txt"))
            thisvar.position = var.name_value("expr_pos")
          when "agg_func"
            @aggFunc = var.name_value("p_txt")	
            thisvar = nil

          else
            raise("unhandled type "+var.name_value("type"))
          end
        end
        return thisvar
      end
    end
    def emit
      myState = TupleSet.new("expr",nil)
      @aggFunc = ""
      # iterate.  create state over projections of (above,termid), Variable/Arg
      eater = VarEater.new

      # the base emitter was sorted
      super.each do |var|
        thisvar = eater.eat(var)
        next if thisvar.nil?
        # here, we don't actually project and aggregate.  we merely append the object to the tuple.
        myVar = var.clone
        myVar.append(Variable.new("var_obj",thisvar.class,thisvar.position,nil),thisvar)
        myState << myVar
      end	
      # sort order should still be preserved.
      return myState
    end
  end	

  class PlanTerm < PlanExpression
    def initialize(runtime, programname,ts, tfuncs)
      super(runtime, programname, ts)
      @tfuncs = tfuncs
    end

    def project_term(tup,*extras)
      key = SchemaKey.new("ruleid","rulename","programid","program_name","delete","termid","term_pos","term_txt",*extras)
      proj = key.project(tup)
    end

    def test_tup_tt(tup,typet)
      typet.each_with_index do |tt,i|
        return false unless tup.schema.contains(Variable.new(tt,String,i,nil))	
      end
      return true
    end

    def accumulate_vars(ts, *type_txt)
      # special function for the base class that all terms can use.
      ## require 'ruby-debug'; debugger
      res = TupleSet.new("terms",nil)
      aHash = Hash.new
      pHash = Hash.new
      # iterate.  state over (etc), Predicate/Selection/Assignment
      ts.each do |var|
        next unless test_tup_tt(var,type_txt)
        projection = project_term(var,*type_txt)
        aHash[projection.to_s] = Array.new if aHash[projection.to_s].nil?
        aHash[projection.to_s] << var.name_value("var_obj")
        pHash[projection.to_s]  = projection

      end
      return pHash,aHash
    end


    #def get_scope(pred)
    #	scopeName = pred.to_s.split("::")
    #	scope = nil
    #	tname = pred
    #	unless (scopeName[1].nil?) then
    #		scope = scopeName[0]
    #		tname = scopeName[1]
    #	end
    #	return [scope,tname]
    #end

    def get_tfuncs(ts)
      #      # require 'ruby-debug'; debugger
      pHash,aHash = accumulate_vars(ts,"tablefunid","termid","function", "nested_predicate_id")

      ret = TupleSet.new("tfuncs",nil)

      pHash.each_key do |k|
        pred = pHash[k]

        # notin, name, event, arguments
        #				# require 'ruby-debug'; debugger if tname == 'strata'
        thistfunc = Function.new(pred.name_value("function"),pred.name_value("nested_predicate_id"))

        ret << proj_cat(pred,thistfunc,"_term_obj",method(:project_term),thistfunc.position)
      end
      return ret
    end

    def get_preds(ts)
      pHash,aHash = accumulate_vars(ts,"pred_txt","event_mod","pred_pos","notin","predicateid")

      tfuncIndx = HashIndex.new(@runtime, @tfuncs,Key.new(3),Integer) if @tfuncs  
      #print "TSchema: #{@tfuncs.tuples.tups[0].schema}\n"
      #      puts "tfuncs: #{tfuncIndx.map.keys.sort.map {|k| k.to_s + " "}}"
      lookups = []
      ret = TupleSet.new("preds",nil)

      pHash.each_key do |k|
        pred = pHash[k]
        (scope,tname) = get_scope(pred.name_value("pred_txt"),@progname)
        case pred.name_value("event_mod") 
        when "insert"
          event = Predicate::Event::INSERT
        when "delete"
          event = Predicate::Event::DELETE
        when "", nil
          event = Predicate::Event::NONE
        else
          raise "unknown event type: #{pred.name_value("event_mod")}\n"
        end

        # notin, name, event, arguments
        #				# require 'ruby-debug'; debugger if tname == 'strata'
        thispred = Predicate.new(pred.name_value("notin"),TableName.new(scope,tname),event,aHash[k])
        # I think the p2 system uses positions starting at 1.
        thispred.set(@runtime,pred.name_value("program_name"),pred.name_value("rulename"),pred.name_value("pred_pos"))

        # wrap pred in TableFunction if there is one
        #       require "ruby-debug"; debugger
        if !tfuncIndx.nil?
          # Debug code: when the index lookup won't find the matching TF, 
          # this loop will find it!
          # hit = nil
          #  @tfuncs.tuples.each do |t|
          #    if t.name_value("nested_predicate_id") == pred.name_value("predicateid")
          #      print "HIT it!  Tup: #{t.to_s}\n"
          #      hit = 1
          #    end
          #  end
          #  # require 'ruby-debug'; debugger if !hit.nil?
          tf = tfuncIndx.lookup(Tuple.new(nil, nil, nil, pred.name_value("predicateid"))) 
          lookups << pred.name_value("predicateid")
          if tf.size > 0
            # this predicate should be wrapped in a TableFunction
            raise "more than one TupleFunction Term matches a Predicate" if tf.size > 1
            #            # require 'ruby-debug'; debugger
            func_tup = tf.tups[0]
            #            puts "YAYYYYYYYY"
            func_name = func_tup.name_value("function")
            thefunc_table = @runtime.catalog.table(TableName.new(Table::GLOBALSCOPE,func_name))
            raise "non-existent TableFunction #{func_name}" if thefunc_table.nil?
            newfunc = Function.new(thefunc_table, thispred)
            newfunc.set(@runtime, pred.name_value("program_name"), pred.name_value("rulename"), pred.name_value("pred_pos"))
            thispred = newfunc
          end
        end

        ret << proj_cat(pred,thispred,"_term_obj",method(:project_term),thispred.position)
      end
      #			puts "lookups: #{lookups.sort.map {|k| k.to_s + " "}}"
      return ret
    end

    def get_assigns(ts)
      #		  # require 'ruby-debug'; debugger
      pHash,aHash = accumulate_vars(ts,"assign_txt","lhs","assign_pos")
      ret = TupleSet.new("assigns",nil)
      pHash.each_key do |k|
        ass = pHash[k]
        lhs_txt = ass.name_value("lhs")
        eval_expr = ass.name_value("assign_txt")
        lhs = Variable.new(lhs_txt,String,0,nil)
        # this is an assignment.  our first variable is the lhs; we don't want to
        # associate this with the expression on the rhs.
        #        # require 'ruby-debug'; debugger # if eval_expr =~ /TableName/
        exprArgs = aHash[k][1..aHash[k].length]
        expr = ArbitraryExpression.new(eval_expr,exprArgs)

        thisassign = Assignment.new(lhs,expr)
        thisassign.set(@runtime,ass.name_value("program_name"),ass.name_value("rulename"),ass.name_value("assign_pos"))
        ret << proj_cat(ass,thisassign,"_term_obj",method(:project_term),thisassign.position)
      end
      return ret
    end
    def get_selects(ts)
      #		  # require 'ruby-debug'; debugger
      pHash,aHash = accumulate_vars(ts,"select_txt","select_pos")
      ret = TupleSet.new("selects",nil)
      pHash.each_key do |k|
        sel = pHash[k]
        eval_expr = sel.name_value("select_txt")
        expr = ArbitraryExpression.new(eval_expr,aHash[k])
        thisselect = SelectionTerm.new(expr)
        thisselect.set(@runtime, @progname,sel.name_value("rulename"),sel.name_value("select_pos"))
        ret << proj_cat(sel,thisselect,"_term_obj",method(:project_term),thisselect.position)
      end
      return ret
    end

    def emit
      ts = super
      #     # require 'ruby-debug'; debugger
      preds = get_preds(ts)
      assigns = get_assigns(ts)
      selects = get_selects(ts)
      tfuncs = get_tfuncs(ts)

      ret = TupleSet.new("terms",nil)
      preds.each { |p| ret << p }
      assigns.each { |a| ret << a }
      selects.each { |s| ret << s }

      sortedRet = TupleSet.new("termsort",nil)
      ret.order_by("programid","ruleid","term_pos") { |t| sortedRet << t } 

      return sortedRet
    end
  end


  class PlanRule < PlanTerm
    def project_rule(tup,*extras)
      key = SchemaKey.new("programid","program_name","ruleid","rulename","delete")
      proj = key.project(tup)
    end

    def accumulate_terms(ts)
      aHash = Hash.new
      pHash = Hash.new
      ts.each do |term|
        proj = project_rule(term)
        pHash[proj.to_s] = proj
        aHash[proj.to_s] = Array.new if aHash[proj.to_s].nil?
        aHash[proj.to_s] << term.name_value("_term_obj")
      end
      return pHash,aHash		
    end
    def emit
      ret = TupleSet.new("rules",nil)
      pHash,aHash = accumulate_terms(super)

      pHash.each_key do |rule|
        context = pHash[rule]

        body = aHash[rule]
        head = body.shift

        d = (context.name_value("delete") == 1)

        ruleObj = Rule.new(1,context.name_value("rulename"),true,d,head,body)
        ruleObj.set(@runtime, context.name_value("program_name"))

        #       # require 'ruby-debug'; debugger if context.name_value("rulename") == 'evaluator'
        ret << proj_cat(pHash[rule],ruleObj,"_rule_obj",method(:project_rule),nil)
      end
      return ret
    end
  end
  class PlanProgram < PlanRule

    def project_program(tup,*extras)
      key = SchemaKey.new("programid","program_name")
      proj = key.project(tup)
    end	
    def emit
      super.each do |r|

      end
    end	
  end

  #############################################################
  def predoftable(table)
    schema = table.schema_of
    p = Predicate.new(false,table.name,table,schema.variables)
    p.set(@runtime,"global","r1",1)
    return p
  end

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

  def program
    return @program
  end

  def thook(name)
    tn = TableName.new(CompilerCatalogTable::COMPILERSCOPE,name)
    # require 'ruby-debug'; debugger if @runtime.nil?
    ret = @runtime.catalog.table(tn)
    raise("parser table #{name} not found in catalog") if ret.nil?

    ret.clear
    return ret
  end

  def catalog_tables
    #this function is heinous: fix later by enumerating these:
    @rules = thook("myRule")
    @terms = thook("myTerm")
    @preds = thook("myPredicate")
    @pexpr = thook("myPrimaryExpression")
    @expr = thook("myExpression")
    @facts = thook("myFact")
    @tables = thook("myTable")
    @columns = thook("myColumn")
    @indices = thook("myIndex")
    @programs = thook("myProgram")
    @assigns = thook("myAssignment")
    @selects = thook("mySelection")
    @tfuncs = thook("myTableFunction")
    @clauses = thook("myClause")
  end

  def initialize(runtime, utterance)
    @runtime = runtime
    # lookup the catalog tables and store references to them in instance variables.
    catalog_tables


    # the choice about whether to pass these references as args, or repeat the lookup above in the compiler, is somewhat arbitrary	
    compiler = OverlogCompiler.new(@runtime, @rules,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices,@programs,@assigns,@selects,@tfuncs,@clauses,@mcalls)


    parseTime  = Benchmark.measure { compiler.parse(utterance) }

    walkTime = Benchmark.measure { compiler.analyze }

    #print "parsetime #{parseTime}\n"
    #print "walktime #{walkTime}\n"

    # now our tables are populated.

    @program = plan_program
  end

  def plan_program
    ts = TupleSet.new("prog",*@programs.tuples)

    # ts points to my programs tuples.  for now, I'm going to assume there's only ever one row in here.
    program = @programs.tuples.tups[0]
    @progname = program.name_value("program_name")
    return Program.new(@runtime,@progname,"your mother")
  end

  def plan

    plan_materializations 
    # that was the old way
    ##plan_rules
    bottom_up

    # facts still new to be planned the new way.
    # it'll be faster and we can remove a ton of artifacts.
    ##plan_facts
    @program.plan
    return @program
  end

  def plan_materializations
    ## require 'ruby-debug'; debugger
    @tables.tuples.each do |table| 
      rescols = join_of(@columns,TupleSet.new("cols",table))
      cols = Array.new
      # SORT!
      rescols.order_by("col_pos") do |col|
        cols << col.name_value("datatype")
      end

      resindx = join_of(@indices,TupleSet.new("cols",table))
      indxs = Array.new	
      emptykey = false
      resindx.order_by("col_pos") do |col|
        x = col.name_value("col_pos")
        if x.nil?
          # empty but existing key means pkey is on all columns
          emptykey = true
        else
          indxs << x 
        end
      end 		

      #      # require 'ruby-debug'; debugger if indxs == []

      typestr = '[' + cols.join(",") + ']'
      indxstr = 'Key.new(' + indxs.join(",") + ')'
      typething = eval(typestr)
      indxthing = eval(indxstr)
      #      indxthing = Key.new(*indxs)

      (scope,tname) = get_scope(table.name_value("tablename"),@progname)
      tn = TableName.new(scope,tname)

      # hackensack
      if indxthing.size == 0 && !emptykey
        # # require 'ruby-debug'; debugger
        # XXXXX assume the lack of pkey indicates an Event table!  
        # Refactor this to be handled by the parser!
        tableObj = EventTable.new(tn,typething)        
      else
        #print "tn=#{tn}\n" 
        tableObj = BasicTable.new(@runtime, TableName.new(scope,tname),indxthing,typething)
      end		
      @program.definition(tableObj)	
      #			# require 'ruby-debug'; debugger if tableObj.name.name == 'strata'	
      @runtime.catalog.register(tableObj)

      mod = table.name_value("watch")
      if (!mod.nil?) then
        0.upto(mod.length-1) do |i|
#          # require 'ruby-debug'; debugger
          watch  = WatchClause.new(1,tn,WatchOp.modifiers(mod[i..i].to_sym))
          watch.set(@runtime, @program.name)
        end
      end
    end
  end

  def add_terms(ts)
    predIndx = HashIndex.new(@runtime, @preds,Key.new(1),Integer)
    assIndx = HashIndex.new(@runtime, @assigns,Key.new(1),Integer)
    selIndx = HashIndex.new(@runtime, @selects,Key.new(1),Integer)

    retSet = TupleSet.new("terms",nil)
    #	  # require 'ruby-debug'; debugger
    ts.each do |tup|
      [predIndx,assIndx,selIndx].each do |i|
        relRec = i.lookup(Tuple.new(nil,tup.name_value("termid")))
        case relRec.tups.size
        when 0 
          # do nothing
        when 1
          newtup = tup.join(relRec.tups[0])
          retSet << newtup
        else
          raise("looked up #{tup.name_value("termid")}, somehow found #{relRec.tups}.  Should only find one match")
        end # case
      end		
    end
    return retSet
  end

  def bu_facts(ts)
    # assumes one program in parse tables at a time!
    #		factIndx = HashIndex.new(@runtime, @facts,Key.new(2),Integer)

    fields = Hash.new
    facts = Hash.new
    ts.order_by("clauseid", "expr_pos") do |tup|
      fields[tup.name_value("tablename")] = Hash.new if fields[tup.name_value("tablename")].nil?

     if fields[tup.name_value("tablename")][[tup.name_value("clauseid"),tup.name_value("arg_pos")]].nil?  
      fields[tup.name_value("tablename")][[tup.name_value("clauseid"),tup.name_value("arg_pos")]] = Array.new 
      fields[tup.name_value("tablename")][[tup.name_value("clauseid"),tup.name_value("arg_pos")]] <<  tup.name_value("expr_text")
      end
    end

#    # require 'ruby-debug'; debugger
    fields.each_key do |tab|
      #lookup = '::'+tab
      #table = Table.find_table(@progname+'::'+tab)

      (scope,tname) = get_scope(tab,@progname)
      table = @runtime.catalog.table(TableName.new(scope,tname))
      raise("#{tab} not found in catalog") if table.nil?

#      # require 'ruby-debug'; debugger
      vars = []; lastk = nil
      (scope,tname) = get_scope(tab,@progname)
      tn = TableName.new(scope,tname)
      ks = fields[tab].keys.sort{|a,b| f = a[0] <=> b[0]; f != 0 ? f : a[1] <=> b[1]}
      ks.each do |k|
        if !lastk.nil? and k[0] != lastk
          fact = Fact.new(1,tn,vars)
          fact.set(@runtime, @program.name)
          puts "FACT: #{fact.to_s}"
          vars = []
        end            
        lastk = k[0]
        if fields[tab][k][0] =~ /\./
          # hack-o-rama!
          # # require 'ruby-debug'; debugger
          vars << eval(fields[tab][k][0]) 
        else
          vars << Value.new(fields[tab][k][0])
        end
      end
      # finish up the last one
      unless vars == []
        # puts fields.to_s
        fact = Fact.new(1,tn,vars)
        fact.set(@runtime, @program.name)
        puts "FACT: #{fact.to_s}"
      end
    end
  end 

  def bottom_up
    # real bottom-up goes out the window, of course, if we have multiple programs in our state tables at once.


    #		# require 'ruby-debug'; debugger
    allPrimaryExpressions = TupleSet.new("pexpr",*@pexpr.tuples)
    allExpressions = indxjoin_of(@expr,"expressionid",allPrimaryExpressions)
    exprTerms = indxjoin_of(@terms,"termid",allExpressions)

    @expr.tuples.each do |t|
      #  print "TERM: #{t}\n"
    end
    @terms.tuples.each do |t|
      #  print "TERM: #{t}\n"
    end
    @preds.tuples.each do |t|
      #  print "PRED: #{t}\n"
    end

    @tfuncs.tuples.each do |t|
      #  print "TF: #{t}\n"
    end
    #print "schema: #{exprTerms.tups[0].schema}\n"
    exprTerms.each do |t|
      #  print "EXPRTM: #{t}\n"
    end

    if @facts.cardinality > 0
      allFacts = indxjoin_of(@facts, "clauseid", @expr.tuples)
      bu_facts(allFacts)
    end
    exprClauses = indxjoin_of(@clauses,"clauseid",exprTerms)
    allRules = indxjoin_of(@rules,"clauseid",exprClauses)
    allPrograms = indxjoin_of(@programs,"programid",allRules)

    fullSet = add_terms(allPrograms)	

    planMaster = PlanProgram.new(@runtime, @progname,fullSet,@tfuncs)
    planMaster.emit.each do |e|
      #puts e
    end

  end

  def OverlogPlanner.scanjoin(tab,ts)
    return join_of(tab,ts)
  end

  def join_of(tab,ts)
    pred = predoftable(tab)
    # performance hurts!  these need to be index joins
    # but unfortunately, since we are moving through the space top-down, we cannot rely 
    # on primary keys for this join.  I don't know yet how to use secondary keys, but we
    # should do so on the FKs to do this search efficiently.
    # top-down is preferable to bottom up because, although we need to scan the whole tree anyway,
    # we have an order of evaluation that we need to follow (materializations, facts, rules)
    # and top-down search avoids repeating inferences.

    if ts.tups[0].nil?
      # require 'ruby-debug'; debugger
      return nil
    end
    sexpr = ScanJoin.new(@runtime, pred,ts.tups[0].schema)
    return sexpr.evaluate(ts)
  end

  def indxjoin_of(tab,key,ts)
    example = ts.tups[0].schema
    pred = predoftable(tab)
    sexpr = IndexJoin.new(@runtime, pred,example,Key.new(example.position(key)),tab.primary)
    return sexpr.evaluate(ts)
  end
end
