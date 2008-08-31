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


class OverlogPlanner
	class PlanProjector
		def initialize(ts)
			@tuples = ts
		end

		def proj_cat(tup,obj,type,func)
			proj = func.call(tup)
			objVar = Variable.new(type,obj.class)
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
				if (!@aggFunc.nil?) then
					if (!var.value("type").eql?("var")) then
						raise("can't aggregate over non-variable ("+var.value("type")+")")
					end
					# fix that string stuff!
					thisvar = Aggregate.new(var.value("p_txt"),@aggFunc,AggregateFunction.type(@aggFunc,String))
					thisvar.position = var.value("expr_pos")
					@aggFunc = nil
				else 
					case var.value("type")
						when "var"
							thisvar = Variable.new(var.value("p_txt"),String)
							thisvar.position = var.value("expr_pos")
						when "const"
							thisvar = Value.new(var.value("p_txt"))
						when "agg_func"
							@aggFunc = var.value("p_txt")	
							thisvar = nil
	
					else
						raise("unhandled type "+var.value("type"))
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
				myVar.append(Variable.new("var_obj",thisvar.class),thisvar)
				myState << myVar
			end	
			# sort order should still be preserved.
			return myState
		end
	end	

	class PlanTerm < PlanExpression
		def project_term(tup,*extras)
			key = SchemaKey.new("ruleid","rulename","programid","program_name","delete","termid","term_pos","term_txt",*extras)
			proj = key.project(tup)
		end

		def test_tup_tt(tup,typet)
			typet.each do |tt|
				return false unless tup.schema.contains(Variable.new(tt,String))	
			end
			return true
		end
		
		def accumulate_vars(ts, *type_txt)
			# special function for the base class that all terms can use.
			res = TupleSet.new("terms",nil)
			aHash = Hash.new
			pHash = Hash.new
			# iterate.  state over (etc), Predicate/Selection/Assignment
			ts.each do |var|
				next unless test_tup_tt(var,type_txt)
				projection = project_term(var,*type_txt)
				aHash[projection.to_s] = Array.new if aHash[projection.to_s].nil?
				aHash[projection.to_s] << var.value("var_obj")
				pHash[projection.to_s]  = projection
		
			end
			return pHash,aHash
		end


		def get_scope(pred)
			scopeName = pred.to_s.split("::")
			scope = nil
			tname = pred
			unless (scopeName[1].nil?) then
				scope = scopeName[0]
				tname = scopeName[1]
			end
			return [scope,tname]
		end
		
		def get_preds(ts)
			pHash,aHash = accumulate_vars(ts,"pred_txt","event_mod","pred_pos")

			ret = TupleSet.new("preds",nil)

			pHash.each_key do |k|
				pred = pHash[k]
				(scope,tname) = get_scope(pred.value("pred_txt"))
				case pred.value("event_mod") 
					when "insert"
						event = Table::Event::INSERT
					when "delete"
						event = Table::Event::DELETE
					when "", nil
						event = Table::Event::NONE
				else
					raise "unknown event type: #{pred.value("event_mod")}\n"
				end
			
				# notin, name, event, arguments
				thispred = Predicate.new(false,TableName.new(scope,tname),event,aHash[k])

				# I think the p2 system uses positions starting at 1.
				thispred.set(pred.value("program_name"),pred.value("rulename"),pred.value("pred_pos"))
				ret << proj_cat(pred,thispred,"_term_obj",method(:project_term))
			end
			return ret
		end
		def get_assigns(ts)
			pHash,aHash = accumulate_vars(ts,"assign_txt","lhs","assign_pos")
			ret = TupleSet.new("assigns",nil)
			pHash.each_key do |k|
				ass = pHash[k]
				lhs_txt = ass.value("lhs")
				eval_expr = ass.value("assign_txt")
				lhs = Variable.new(lhs_txt,String)
				lhs.position = 0
				expr = ArbitraryExpression.new(eval_expr,aHash[k])

				thisassign = Assignment.new(lhs,expr)
				thisassign.set(ass.value("program_name"),ass.value("rulename"),ass.value("assign_pos"))
				ret << proj_cat(ass,thisassign,"_term_obj",method(:project_term))
			end
			return ret
		end
		def get_selects(ts)

			pHash,aHash = accumulate_vars(ts,"select_txt","select_pos")
			ret = TupleSet.new("selects",nil)
			pHash.each_key do |k|
				sel = pHash[k]
				eval_expr = sel.value("select_txt")
				expr = ArbitraryExpression.new(eval_expr,aHash[k])
				thisselect = SelectionTerm.new(expr)
				thisselect.set(@progname,sel.value("rulename"),sel.value("select_pos"))
				ret << proj_cat(sel,thisselect,"_term_obj",method(:project_term))
			end
			return ret
		end
		def emit
			ts = super
			preds = get_preds(ts)
			assigns = get_assigns(ts)
			selects = get_selects(ts)

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
				aHash[proj.to_s] << term.value("_term_obj")
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

                        	d = context.value("delete").eql?("1")
                        	ruleObj = Rule.new(1,context.value("rulename"),true,d,head,body)
                        	ruleObj.set(context.value("program_name"))

				ret << proj_cat(pHash[rule],ruleObj,"_rule_obj",method(:project_rule))
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
		p.set("global","r1",1)
		return p
	end

	def get_scope(pred)
		scopeName = pred.to_s.split("::")
		scope = nil
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
		tn = TableName.new("global",name)
		ret = Table.find_table(tn)
		raise("parser table #{name} not found in catalog") if ret.nil?
		return ret
	end

	def catalog_tables
		@rules = thook("MyRule")
		@terms = thook("MyTerm")
		@preds = thook("MyPredicate")
		@pexpr = thook("MyPrimaryExpression")
		@expr = thook("MyExpression")
		@facts = thook("MyFact")
		@tables = thook("MyTable")
		@columns = thook("MyColumn")
		@indices = thook("MyIndex")
		@programs = thook("MyProgram")
		@assigns = thook("MyAssignment")
		@selects = thook("MySelection")
		@tfuncs = thook("MyTableFunction")
	end

	def initialize(utterance)
		# lookup the catalog tables and store references to them in instance variables.
		catalog_tables
		
		# the choice about whether to pass these references as args, or repeat the lookup above in the compiler, is somewhat arbitrary	
		compiler = OverlogCompiler.new(@rules,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices,@programs,@assigns,@selects,@tfuncs)
                compiler.parse(utterance)

                compiler.analyze
	
		# now our tables are populated.

		@program = plan_program
	end

	def plan_program
		ts = TupleSet.new("prog",*@programs.tuples)

		# ts points to my programs tuples.  for now, I'm going to assume there's only ever one row in here.
		program = ts.tups[0]
		@progname = program.value("program_name")
		return Program.new(@progname,"your mother")
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
		@tables.tuples.each do |table| 

			rescols = join_of(@columns,TupleSet.new("cols",table))
			cols = Array.new
			# SORT!
			rescols.order_by("col_pos") do |col|
				cols << col.value("datatype")
			end
			
			resindx = join_of(@indices,TupleSet.new("cols",table))
			indxs = Array.new	
			resindx.order_by("col_pos") do |col|
				indxs << col.value("col_pos")
			end 		

			typestr = '[' + cols.join(",") + ']'
			indxstr = 'Key.new(' + indxs.join(",") + ')'
			typething = eval(typestr)
			indxthing = eval(indxstr)

			(scope,tname) = get_scope(table.value("tablename"))
		
			# TABLE TYPE AND LIFETIME UNIMPLEMENTED...
			#table = BasicTable.new(TableName.new(scope,tname),Table::INFINITY, Table::INFINITY,indxthing,typething)

			# hackensack
			tn = table.value("tablename")
			if tn.eql?("periodic") then
				print "periodic, sucker!\n"
				table = EventTable.new(TableName.new(nil,tn),typething)
			else
				#print "tn=#{tn}\n" 
				(scope,tname) = get_scope(tn)
				table = RefTable.new(TableName.new(scope,tname),indxthing,typething)
			end
			
			@program.definition(table)			
		end
	end

	def add_terms(ts)
		predIndx = HashIndex.new(@preds,Key.new(1),Integer)
		assIndx = HashIndex.new(@assigns,Key.new(1),Integer)
		selIndx = HashIndex.new(@selects,Key.new(1),Integer)

		retSet = TupleSet.new("terms",nil)
		ts.each do |tup|
			[predIndx,assIndx,selIndx].each do |i|
				relRec = i.lookup(Tuple.new(nil,tup.value("termid")))
				case relRec.tups.size
					when 0 
						# do nothing
					when 1
						retSet << tup.join(relRec.tups[0])
				else
					raise("bad; more than one term matching termid")
				end
			end
		end
		return retSet
	end

	def bu_facts(ts)
		# assumes one program in parse tables at a time!
		factIndx = HashIndex.new(@facts,Key.new(1),Integer)

		fields = Hash.new
		ts.order_by("termid","expr_pos") do |tup|
			relRec = factIndx.lookup(Tuple.new(nil,tup.value("termid")))
			case relRec.size
				when 0:
					next
				when 1:
					exemplary = relRec.tups[0]
			else
				raise("duplicate fact?")
			end

			fields[exemplary.value("tablename")] = Hash.new if fields[exemplary.value("tablename")].nil?
			fields[exemplary.value("tablename")][tup.value("termid")] = Array.new if fields[exemplary.value("tablename")][tup.value("termid")].nil?
      unless tup.value("type").eql?("const")
        require 'ruby-debug'; debugger
			  raise("fact args must be constants, instead is type " + tup.value("type").to_s)
		  end
			fields[exemplary.value("tablename")][tup.value("termid")] <<  tup.value("p_txt")
		end
		fields.each_key do |tab|
			lookup = '::'+tab
			table = Table.find_table('::'+tab)
			raise("#{tab} not found in catalog") if table.nil?

			fields[tab].keys.sort.each  do |term|
				table.insert(TupleSet.new("fact",Tuple.new(*fields[tab][term])),nil)
			end
		end
	end 

	def bottom_up
		# real bottom-up goes out the window, of course, if we have multiple programs in our state tables at once.


		
		allPrimaryExpressions = TupleSet.new("pexpr",*@pexpr.tuples)
		allExpressions = indxjoin_of(@expr,"expressionid",allPrimaryExpressions)
		allTerms = indxjoin_of(@terms,"termid",allExpressions)


		bu_facts(allTerms)
		allRules = indxjoin_of(@rules,"ruleid",allTerms)
		allPrograms = indxjoin_of(@programs,"programid",allRules)

		fullSet = add_terms(allPrograms)	

		planMaster = PlanProgram.new(fullSet)
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

		sexpr = ScanJoin.new(pred,ts.tups[0].schema)
		return sexpr.evaluate(ts)
	end

	def indxjoin_of(tab,key,ts)
		example = ts.tups[0].schema
		pred = predoftable(tab)
		sexpr = IndexJoin.new(pred,example,Key.new(example.position(key)),tab.primary)
		return sexpr.evaluate(ts)
	end
end
