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


# had to.  will revisit.
def suck_nums(node)
	#puts node.inspect
	returns  = Array.new
	if (defined? node.DecimalNumeral) then
		returns << node.DecimalNumeral.text_value
	else 
		returns = Array.new
		if (!node.elements.nil?) then
			node.elements.each do |e|
				list = suck_nums(e)
				if (!list.empty?) then
					returns.concat(list)
				end
			end	
		end
	end
	return returns
end


# local modules

class VisitGeneric < Treewalker::Handler
	def initialize
		@@ids["_Object"] = -1
	end 
	def semantic(text,obj)
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
	def semantic(text,obj)
		super(text,obj)

		@@ids["_Object"] = @@ids["_Object"] + 1
		predtab(@@state["Rule"],@@state["Predicate"],@@ids["_Object"]);
		@@ids["PrimaryExpression"] = -1
		objtab(@@state["Rule"],@@ids["_Object"],@@state["Predicate"],"predicate")
	end
end

class VisitConstant < VisitGeneric
	def semantic(text,obj)
		#print "constant "+text+"\n"
		#print "type is "+text.class.to_s+"\n"
		super(text,obj)
		## WHOA!
		t = text.gsub('"',"")
		#termtab(@@state["Rule"],@@state["_Object"],@@state["PrimaryExpression"],text,'"const"')
		termtab(@@state["Rule"],@@ids["_Object"],@@state["PrimaryExpression"],t,'"const"')
		
	end
end

class VisitVariable < VisitGeneric
	def semantic(text,obj)
		#print "Variable ("+text+")\n"
		#print "rule "+@@state["orule"] +",pred "+@@state["head"]+" Variable "+text+"\n"

		super(text,obj)
		termtab(@@state["Rule"],@@ids["_Object"],@@state["PrimaryExpression"],text,'"var"')
		# insert into term table
	end
end

class VisitFact < VisitGeneric
	def semantic(text,obj)
		#print "Fact: 
		super(text,obj)
		facttab(text,@@ids["_Object"])
		@@ids["PrimaryExpression"] = -1 ###= @@ids["Predicate"] = -1
		@@ids["_Object"] = @@ids["_Object"] + 1
	end
end

class VisitRule < VisitGeneric
	def semantic(text,obj)
		
		t = text.gsub('"',"")
		super(t,obj)
		ruletab(@@state["Rule"])
		@@ids["Predicate"] = @@ids["PrimaryExpression"] = @@ids["_Object"] = -1
	end
end

class VisitTable < VisitGeneric
	def semantic(text,obj)
		super(text,obj)
		#print obj.Keys.inspect
		#print "TABLE("+text+"\n"
		tabtab(@@state["TableName"])
		#@@ids["_Object"] = @@ids["_Object"] + 1
	end
end

class VisitColumn < VisitGeneric
	def semantic(text,obj)
		super(text,obj)	
		#print "\tCOL: "+text+"\n"	
		coltab(@@ids["TableName"],@@ids["Type"],text);
	end
end
class VisitIndex < VisitGeneric
	def semantic(text,obj)
		super(text,obj)
		suck_nums(obj).each do |indx|
			#print "\tINDEX: "+indx+"\n"
			indxtab(@@ids["TableName"],indx)
		end
	end
end



class VisitAssignment < VisitGeneric
	def semantic(text,obj)
		@@ids["_Object"] = @@ids["_Object"] + 1
		@@ids["PrimaryExpression"] = -1
	
			
		t = obj.Variable.text_value.gsub('"','\"')

		objtab(@@state["Rule"],@@ids["_Object"],@@state["Assignment"],"assignment")
		assigntab(@@state["Rule"],@@ids["_Object"],t)
	end
end


class VisitSelection < VisitGeneric
	def semantic(text,obj)
		@@ids["_Object"] = @@ids["_Object"] + 1
		@@ids["PrimaryExpression"] =  -1
		objtab(@@state["Rule"],@@ids["_Object"],@@state["Selection"],"selection")
		t = text.gsub('"','\"')
		selecttab(@@state["Rule"],@@ids["_Object"],t)
	end
end


class VisitExpression < VisitGeneric
	def semantic(text,obj)
		#puts obj.inspect
		if (!defined? obj.PrimaryExpression) then
			# this is no good!!!
			old =@@state["PrimaryExpression"]
			@@state["PrimaryExpression"] = [old[0],old[1]+1]
			termtab(@@state["Rule"],@@ids["_Object"],@@state["PrimaryExpression"],text,'"expr"')
			@@ids["_Object"] = @@ids["_Object"] + 1
			@@ids["PrimaryExpression"] =  -1
			objtab(@@state["Rule"],@@ids["_Object"],@@state["Selection"],"expression")
			t = text.gsub('"','\"')
			exprtab(@@state["Rule"],@@ids["_Object"],t)
		end
	end
end


