require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
#require 'lib/lang/compiler.rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"
require "lib/lang/plan/value.rb"



class OverlogPlanner


	def predoftable(table)
	        schema = table.schema_of
	        p = Predicate.new(false,table.name,table,schema.variables)
		p.set("global","r1",1)
		return p
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

		puts @program
		#require 'ruby-debug'; debugger
		@program.plan

		#puts @program.get_query(Tablename.new(nil,"path"))
		return @program
	end

	def plan_materializations
		@tables.tuples.each do |table| 
			p_cols = predoftable(@columns)

			scols = ScanJoin.new(p_cols,table.schema)
			rescols = scols.evaluate(TupleSet.new("cols",table))

			cols = Array.new
			# SORT!
			rescols.tups.each do |col|
				cols << col.value("datatype")
			end
			
			p_indx = predoftable(@indices)
			sindx = ScanJoin.new(p_indx,table.schema)
			resindx = sindx.evaluate(TupleSet.new("cols",table))
		
			indxs = Array.new	
			resindx.each do |col|
				indxs << col.value("col_pos")
			end 		

			typestr = '[' + cols.join(",") + ']'
			indxstr = 'Key.new(' + indxs.join(",") + ')'


			typething = eval(typestr)
			indxthing = eval(indxstr)
			
			table = BasicTable.new(TableName.new(nil,table.value("tablename").to_s),Table::INFINITY, Table::INFINITY,indxthing,typething)
			@program.definition(table)			
		end
	end

	def plan_rules

		p_rule = predoftable(@rules)
	
		sj = ScanJoin.new(p_rule,@programs.schema_of)
		ts = TupleSet.new("prog",*@programs.tuples)
		# save res as a member variable: I'll want to reuse it.
		res = sj.evaluate(ts)

		res.tups.each do |rule|
		
			p_term = predoftable(@terms)
			sterm = ScanJoin.new(p_term,rule.schema)
			resterm = sterm.evaluate(TupleSet.new("rule",rule))
			
			rulename = rule.value("rulename")
			body = plan_preds(resterm,rulename)
			head = body.shift
			assigns = plan_assignments(resterm,rulename)	
			assigns.each do |a|
				print "\t\t"+a.inspect+"\n"
				body << a	
			end
		
			# location?  extract rulename!  isPublic, isDelete
			rule = Rule.new(1,rulename,true,false,head,body)
			rule.set(@progname)
			puts rule.inspect
			print "------------\n"
			puts rule.to_s
			print "============\n"
		end
	end
	def plan_preds(resterm,rulename)
		# resterm is a joinable resultset of terms for the current rule.

		p_pred = predoftable(@preds)
		spred = ScanJoin.new(p_pred,resterm.tups[0].schema)
		respred = spred.evaluate(resterm)
	
		predicates = Array.new
		respred.tups.each do |pred|
			# skip the head, for now
		
			# a row for each predicate.  let's grab the vars
			p_expr = predoftable(@expr)
			sexpr = ScanJoin.new(p_expr,pred.schema)
			resexpr = sexpr.evaluate(TupleSet.new("p",pred))
			p_var = predoftable(@pexpr)
			spexpr = ScanJoin.new(p_var,resexpr.tups[0].schema)
			respexpr = spexpr.evaluate(resexpr)

			args = Array.new
			respexpr.tups.each do |var|
				if (var.value("type").eql?("var")) then
					thisvar = Variable.new(var.value("p_txt"),String)
					thisvar.position = var.value("expr_pos")
				elsif (var.value("type").eql?("const")) then
					thisvar = Value.new(var.value("p_txt"))
				else
					#function?
					raise("unhandled type "+var.value("type"))
				end
				args << thisvar
			end
			thispred = Predicate.new(false,TableName.new(nil,pred.value("pred_txt")),Table::Event::NONE,args)
			# 2 things about the below.  1.) get rulenames working!  2.) I think the p2 system uses positions starting at 1.
			thispred.set(@progname,rulename,pred.value("pred_pos"))
			predicates << thispred
		end
		return predicates
	end

	def plan_assignments(resterm,rulename)
		# resterm is a joinable resultset of terms for the current rule.

	
		p_assign = predoftable(@assigns)
		sassign = ScanJoin.new(p_assign,resterm.tups[0].schema)
		resassign = sassign.evaluate(resterm)
	
		assignments = Array.new

		resassign.tups.each do |ass|
			lhs_txt = ass.value("lhs")
			eval_expr = ass.value("assign_txt")

			#print "eval_expr is "+eval_expr+"->>"+lhs+"\n"
	
			p_expr = predoftable(@expr)
			sexpr = ScanJoin.new(p_expr,ass.schema)
			resexpr = sexpr.evaluate(TupleSet.new("p",ass))
			p_var = predoftable(@pexpr)
			spexpr = ScanJoin.new(p_var,resexpr.tups[0].schema)
			respexpr = spexpr.evaluate(resexpr)

			args = Array.new
			respexpr.tups.each do |var|
				if (var.value("type").eql?("var")) then
					# danger: all vars are strings....
					thisvar = Variable.new(var.value("p_txt"),String)
					thisvar.position = var.value("expr_pos")
					args << thisvar
				elsif (var.value("type").eql?("const")) then
					# consts are already in the expression!
					#thisvar = Value.new(var.value("p_txt"))
				else
					#function?
					raise("unhandled type "+var.value("type"))
				end
			end
			
			lhs = Variable.new(lhs_txt,String)
			lhs.position = 0
			expr = ArbitraryExpression.new(eval_expr,args)
			thisassign = Assignment.new(lhs,expr)
			thisassign.set(@progname,rulename,ass.value("assign_pos"))
			assignments << thisassign
		end
		return assignments 
	end


end


