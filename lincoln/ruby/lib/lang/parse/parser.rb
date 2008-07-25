#!/usr/bin/ruby

require "rubygems"
require "treetop"
require "core.rb"
#require "olg.rb"

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'termtab'

require "Treewalker.rb"
require 'local_tw.rb'
require 'moutput.rb'

verbose = 'n'

prog = ''
while line = STDIN.gets
	prog = prog + line
end

prog = "program foo;\nfoo(A,B) :- bar(A,B);\n"

preds = PredicateTable.new
terms = TermTable.new
pexpr = PrimaryExpressionTable.new
expr = ExpressionTable.new
#rule = Rule::RuleTable.new
#table = BasicTable.new

#terms = pexpr = nil

compiler = OverlogCompiler.new(nil,terms,preds,pexpr,expr)
compiler.parse(prog)
compiler.analyze()

puts terms.to_s
print "=====================\n"

puts preds.to_s
print "=====================\n"
puts expr.to_s
print "=====================\n"

puts pexpr.to_s
print "=====================\n"
print "=====================\n"
