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



class OverlogPlanner


	def predoftable(table)
	        schema = table.schema_of
	        p = Predicate.new(false,table.name,table,schema.variables)
		p.set("global","r1",1)
		return p
	end


	def initialize(utterance,rules,terms,preds,pexps,exps,facts,tables,columns,indices,programs)
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

			
		compiler = OverlogCompiler.new(@rules,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices,@programs)
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
		print "progname is "+@progname+"\n"
		return Program.new(@progname,"your mother")
	end

	def plan
		plan_materializations
		plan_rules

		@program.plan
		@program.queries.each do |query|
			print "QUERY\n"
			puts query.inspect
	
		end
		return @program
	end

	def plan_materializations
		@tables.tuples.each do |table| 
			print "\ttable "+table.value("tablename").to_s+"\n"
			p_cols = predoftable(@columns)

			scols = ScanJoin.new(p_cols,table.schema)
			rescols = scols.evaluate(TupleSet.new("cols",table))

			cols = Array.new
			# SORT!
			rescols.each do |col|
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

			print "typestr "+typestr +", indxstr "+indxstr+"\n"

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

		
			p_pred = predoftable(@preds)
			spred = ScanJoin.new(p_pred,resterm.tups[0].schema)
			respred = spred.evaluate(resterm)
		
			body = Array.new
			head = nil
			respred.tups.each do |pred|
				# skip the head, for now

				print "predicate "+pred.value("pred_txt").to_s+"\n"
				
				# a row for each predicate.  let's grab the vars
				p_expr = predoftable(@expr)
				sexpr = ScanJoin.new(p_expr,pred.schema)
				resexpr = sexpr.evaluate(TupleSet.new("p",pred))
				p_var = predoftable(@pexpr)
				spexpr = ScanJoin.new(p_var,resexpr.tups[0].schema)
				respexpr = spexpr.evaluate(resexpr)
	
				args = Array.new
				respexpr.tups.each do |var|
					print "\t"+var.value("p_txt")+"\n"
					thisvar = Variable.new(var.value("p_txt"),String)
					thisvar.position = var.value("p_pos")
					args << thisvar
				end
				thispred = Predicate.new(false,TableName.new(nil,pred.value("pred_txt")),Table::Event::NONE,args)
				thispred.set(@progname,"r2",1)
				if (pred.value("pred_pos") == 0) then
					head = thispred
				else
					body << thispred
				end
			end

		
			# location?  extract rulename!  isPublic, isDelete
			rule = Rule.new(1,"r2",true,false,head,body)
			rule.set(@progname)
			puts rule.inspect
		end
	end

end
