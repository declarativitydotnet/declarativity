require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'
require 'lib/lang/parse/schema.rb'
require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"
require "lib/lang/plan/value.rb"
require "lib/lang/plan/arbitrary_expression.rb"
require "lib/types/function/aggregate_fn.rb"


class OverlogPlanner
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

	def initialize(utterance,rules,terms,preds,pexps,exps,facts,tables,columns,indices,programs,assigns,selects)
                @rules = rules
                @terms = terms
                @preds = preds
                @pexpr = pexps
                @expr = exps
                @facts = facts
                @tables = tables
                @columns = columns
                @indices = indices
                @programs = programs
		@assigns = assigns
		@selects = selects

			
		compiler = OverlogCompiler.new(@rules,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices,@programs,@assigns,@selects)
                #compiler.verbose = 'y'
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
		plan_rules
		#bottom_up

		#return		

		plan_facts
		@program.plan


		return @program
	end

	def plan_facts
		@facts.tuples.each do |fact|
			tab = fact.value("tablename")
			lookup = '::'+tab
			table = Table.find_table('::'+tab)
			raise("#{tab} not found in catalog") if table.nil?

			resterm = join_of(@terms,TupleSet.new("fact",fact))
			resterm.each do |t| 
				vars = get_vars(t)
				varNames = Array.new
				vars.each do |v|
					varNames << v.value
				end
				table.insert(TupleSet.new("fact",Tuple.new(*varNames)),nil)
			end
		end
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
			table = BasicTable.new(TableName.new(scope,tname),Table::INFINITY, Table::INFINITY,indxthing,typething)
			
			@program.definition(table)			
		end
	end

	def bottom_up
		# real bottom-up goes out the window, of course, if we have multiple programs in our state tables at once.
		
		#set = join_of(@expr,
		#		join_of(@terms,
		#			join_of(@programs,TupleSet.new("rules",*@rules.tuples))))
	
		#set = indxjoin_of(@terms,

	
	#	set =	indxjoin_of(@terms,@pexpr.schema.size + MyExpressionTable::Field::TERMID,
	#			indxjoin_of(@expr,MyPrimaryExpressionTable::Field::EXPRESSIONID,TupleSet.new("pexpr",*@pexpr.tuples)))

		allPrimaryExpressions = TupleSet.new("pexpr",*@pexpr.tuples)
		allExpressions = indxjoin_of(@expr,"expressionid",allPrimaryExpressions)
		allTerms = indxjoin_of(@terms,"termid",allExpressions)
		allRules = indxjoin_of(@rules,"ruleid",allTerms)
		allPrograms = indxjoin_of(@programs,"programid",allRules)
		

		#allPreds = join_of(@preds,allTerms)
		plan_pred
		
		

		set = allPrograms

		#set =	join_of(@expr,TupleSet.new("pexpr",*@pexpr.tuples))
		puts set.tups[0].schema
		set.each do |s|
			puts s.to_s
		end

		
		#puts set
		
	end

	def plan_bu_preds(set)
		allPreds = join_of(@preds,allTerms)

		

	end

	def plan_rules

		# save res as a member variable: I'll want to reuse it.
		res = join_of(@rules,TupleSet.new("prog",*@programs.tuples))

		# need to put an ordering over the rules!
		res.tups.each do |rule|
			resterm = join_of(@terms,TupleSet.new("rule",rule))
			rulename = rule.value("rulename")
			body = plan_preds(resterm,rulename)

			head = body.shift
			assigns = plan_assignments(resterm,rulename)	
			assigns.each do |a|
				body << a	
			end
			selects = plan_selections(resterm,rulename)
			selects.each do |s|
				body << s
			end
		
			# location? rulename, isPublic, isDelete, head, body
			d = rule.value("delete").eql?("1")
			rule = Rule.new(1,rulename,true,d,head,body)
			rule.set(@progname)
		end
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

	def get_vars(it)	
			resexpr = join_of(@expr,TupleSet.new("p",it))
			respexpr = join_of(@pexpr,resexpr)

			args = Array.new
			aggFunc = ""
			respexpr.order_by("p_pos") do |var|
				if (!aggFunc.eql?("")) then
					if (!var.value("type").eql?("var")) then
						raise
					end
					# fix that string stuff!
					thisvar = Aggregate.new(var.value("p_txt"),aggFunc,AggregateFunction.type(aggFunc,String))
					thisvar.position = var.value("expr_pos")
					aggFunc = ""
				else 
					case var.value("type")
						when "var"
							thisvar = Variable.new(var.value("p_txt"),String)
							thisvar.position = var.value("expr_pos")
						when "const"
							thisvar = Value.new(var.value("p_txt"))
						when "agg_func"
							aggFunc = var.value("p_txt")	
							next
		
					else
						raise("unhandled type "+var.value("type"))
					end
				end
				args << thisvar
			end
		return args
	end

	def plan_preds(resterm,rulename)
		# resterm is a joinable resultset of terms for the current rule.

		respred = join_of(@preds,resterm)
		predicates = Array.new
		#respred.order_by("term_pos") do |pred|
		respred.each do |pred|
			# skip the head, for now
		
			# a row for each predicate.  let's grab the vars
			args = get_vars(pred)
			(scope,tname) = get_scope(pred.value("pred_txt"))

			case pred.value("event_mod") 
				when "insert"
					event = Table::Event::INSERT
				when "delete"
					event = Table::Event::DELETE
				when ""
					event = Table::Event::NONE
			else
				raise "unknown event type: #{pred.value("event_mod")}\n"
			end
			
			# notin, name, event, arguments
			thispred = Predicate.new(false,TableName.new(scope,tname),event,args)
			# I think the p2 system uses positions starting at 1.
			thispred.set(@progname,rulename,pred.value("pred_pos"))
			predicates << thispred
		end
		return predicates
	end

	def plan_assignments(resterm,rulename)
		resassign = join_of(@assigns,resterm)
	
		assignments = Array.new
		resassign.order_by("assign_pos") do |ass|
			lhs_txt = ass.value("lhs")
			eval_expr = ass.value("assign_txt")
			args = get_vars(ass)
			lhs = Variable.new(lhs_txt,String)
			lhs.position = 0
			expr = ArbitraryExpression.new(eval_expr,args)

			thisassign = Assignment.new(lhs,expr)
			thisassign.set(@progname,rulename,ass.value("assign_pos"))
			assignments << thisassign
		end
		return assignments 
	end

	def plan_selections(resterm,rulename)
		resselect = join_of(@selects,resterm)
	
		selections = Array.new
		resselect.order_by("select_pos") do |sel|
			eval_expr = sel.value("select_txt")
			args = get_vars(sel)
			expr = ArbitraryExpression.new(eval_expr,args)
			thisselect = SelectionTerm.new(expr)
			thisselect.set(@progname,rulename,sel.value("select_pos"))
			selections << thisselect
		end
		return selections 
	end

end


