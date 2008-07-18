# Treewalker callbacks specific to parsing Overlog programs

require "rubygems"
require "treetop"
require "core.rb"
#require "olg.rb"

require "Treewalker.rb"


# um... symbol tables?
@@state = Hash.new
@@ids = Hash.new


@@predicates = Predicate::PredicateTable.new


# local modules

class VisitGeneric < Treewalker::Handler
	def initialize
		@@ids["_Object"] = 0
	end 
	def semantic(text)
		#print "generic semantic action for "+self.token+" =  "+text+"\n"
		#print "set "+self.token+" = "+text+"\n"
		if (@@ids[self.token].nil?) then
			@@ids[self.token] = 0
		else
			@@ids[self.token] = @@ids[self.token] + 1
		end
		@@state[self.token] = [text,@@ids[self.token]]
	end
end

class VisitPredicate < VisitGeneric
	def semantic(text)
		super(text)
		predtab(@@state["Rule"],@@state["Predicate"],@@ids["_Object"]);
		@@ids["PrimaryExpression"] = 0
		@@ids["_Object"] = @@ids["_Object"] + 1
		#@@state["_Object"] == [text,@@ids["_Object"]]
	end
end

class VisitConstant < VisitGeneric
	def semantic(text)
		#print "constant "+text+"\n"
		#print "type is "+text.class.to_s+"\n"
		super(text)
		#termtab(@@state["Rule"],@@state["_Object"],@@state["PrimaryExpression"],text,'"const"')
		termtab(@@state["Rule"],@@ids["_Object"],@@state["PrimaryExpression"],text,'"const"')
	end
end

class VisitVariable < VisitGeneric
	def semantic(text)
		#print "Variable ("+text+")\n"
		#print "rule "+@@state["orule"] +",pred "+@@state["head"]+" Variable "+text+"\n"

		super(text)
		termtab(@@state["Rule"],@@ids["_Object"],@@state["PrimaryExpression"],text,'"var"')
		# insert into term table
	end
end

class VisitFact < VisitGeneric
	def semantic(text)
		#print "Fact: 
		super(text)
		facttab(text,@@ids["_Object"])
		@@ids["PrimaryExpression"] = 0 ###= @@ids["Predicate"] = 0
		@@ids["_Object"] = @@ids["_Object"] + 1
	end
end

class VisitRule < VisitGeneric
	def semantic(text)
		super(text)
		ruletab(@@state["Rule"])
		@@ids["Predicate"] = @@ids["PrimaryExpression"] = @@ids["_Object"] = 0
	end
end

class VisitTable < VisitGeneric
	def semantic(text)
		print "Fact: "
		super(text)
		tabtab(@@state["table"])
		@@ids["_Object"] = @@ids["_Object"] + 1
	end
end

class VisitAssignment < VisitGeneric
	def semantic(text)
		#print "Variable "+text+"\n"
		#print "rule "+@@state["orule"] +", pred "+@@state["head"]+" Variable "+text+"\n"
		#insert into term table
		print "Assignment: "+text+"\n"
		@@ids["_Object"] = @@ids["_Object"] + 1
		@@ids["PrimaryExpression"] = 0 
		#@@state["_Object"] == [text,@@ids["_Object"]]
	end
end


class VisitSelection < VisitGeneric
	def semantic(text)
		print "Selection: "+text+"\n"
		@@ids["_Object"] = @@ids["_Object"] + 1
		@@ids["PrimaryExpression"] = 0 
	end
end


