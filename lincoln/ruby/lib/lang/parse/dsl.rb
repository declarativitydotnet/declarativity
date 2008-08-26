#!/usr/bin/ruby

# convert SQL-DDL-style "create table" statements to ruby ObjectTable classes with equivalent definitions.
# ie ruby dsl.rb < create.sql > schema.rb

require "rubygems"
require "treetop"
require 'tree_walker.rb'
require 'ddl.rb'

$current = Hash.new
$tables = Hash.new
$types = Hash.new
$keys = Hash.new

class Visit < TreeWalker::Handler
	def semantic(text,obj)
		$current[self.token] = text
	end
end

class VTable < Visit
	def semantic(text,obj)
		$tables[text] = Array.new
		$types[text] = Array.new
		$keys[text] = Array.new
		$position = 0
		super(text,obj)
	end
end

class VCol < Visit
	def semantic(text,obj)
		#print "current of "+$current["tablename"]+"\n"
		$tables[$current["tablename"]] << text.delete('+')
		$position += 1
		super(text,obj)
	end 
end

class VKey < Visit
  def semantic(text,obj)
    $keys[$current["tablename"]] << $position-1
    super(text,obj)
  end
end

class VType < Visit
	def semantic(text,obj)
		super(text,obj)
		$types[$current["tablename"]] << text
	end
end

class VNum < Visit
	def semantic(text,obj)
		super(text,obj)
		$keys[$current["tablename"]] << text
	end
		
end


prog = ''
while line = STDIN.gets
	prog = prog + line
end


parser = DdlParser.new
tree = parser.parse(prog)
if !tree
      puts 'failure'
     raise RuntimeError.new(parser.failure_reason)
end

sky = TreeWalker.new(tree)

v = Visit.new

sky.add_handler("tablename",VTable.new,1)
sky.add_handler("key_colname",VCol.new,1)
sky.add_handler("key_modifier",VKey.new,1)
sky.add_handler("dtype",VType.new,1)


sky.add_handler("num",VNum.new,1)

sky.walk("n")

print "require 'lib/types/table/object_table'\n"
print "require 'lib/lang/parse/compiler_mixins'\n"
# print "require 'lib/lang/plan/predicate'\n"
# print "require 'lib/lang/plan/selection_term'\n"
# print "require 'lib/lang/plan/program'\n"
$tables.sort.each do |table, arr|
  tableCap = table[0..0].capitalize + table[1..table.length]
  mixin = tableCap+"TableMixin"
	print "class "+tableCap+"Table < ObjectTable\n"
  print "include "+mixin+" if defined? "+mixin+"\n"
	if ($keys[table].size > 0) then
		print "\t@@PRIMARY_KEY = Key.new("+$keys[table].join(",")+")\n"
	else
		# print "\t@@PRIMARY_KEY = Key.new("+(0..arr.size-1).to_a.join(",")+")\n"
		print "\t@@PRIMARY_KEY = Key.new\n"
	end

	print "\tclass Field\n"
	(0..arr.size-1).each do |i|
		print "\t\t"+arr[i].upcase+"="+i.to_s+"\n"
	end
	print "\tend\n"
	print "\t@@SCHEMA = ["+$types[table].join(",")+"]\n"

	print "\n\tdef initialize\n"
        print "\t\tsuper(TableName.new(GLOBALSCOPE, \""+table+"\"), @@PRIMARY_KEY,  TypeList.new(@@SCHEMA))\n"
        print "\t\tif defined? "+mixin+" and "+mixin+".methods.include? 'initialize_mixin'\n\t\t\t then initialize_mixin \n\t\tend\n"
  # print "\t\tprogramKey = Key.new(Field::" + arr[0].upcase+")\n"
  # print "\t\tindex = HashIndex.new(self, programKey, Index::Type::SECONDARY)\n"
  # print "\t\t@secondary[programKey] = index\n"
	print "\tend\n"

  print "\n\tdef field(name)\n"
  print "\n\t\teval('Field::'+name)\n"
  print "\n\tend"
  
  print "\n\tdef scope\n"
  print "\n\t\tGLOBALSCOPE\n"
  print "\n\tend"
  
  print "\n\tdef pkey\n"
  print "\n\t\t@@PRIMARY_KEY\n"
  print "\n\tend"

  print "\n\tdef schema\n"
  print "\n\t\t@@SCHEMA\n"
  print "\n\tend"
  
	print "\n\tdef schema_of\n"
	(0..arr.size-1).each do |i|
		print "\t\t"+ arr[i]+" = Variable.new(\""+arr[i]+"\","+$types[table][i]+")\n"
		print "\t\t"+arr[i]+".position="+i.to_s+"\n"
	end
	print "\t\treturn Schema.new(\""+tableCap+"\",["+arr.join(",")+"])\n"
	print "\tend\n"
	print "end\n\n"
end
