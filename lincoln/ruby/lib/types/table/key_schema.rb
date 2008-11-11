require "lib/types/table/key"

class SchemaKey < Key
	def project(tup)
		projection = super(tup)

		items = Array.new
		i = 0
		@attributes.each do |a|
			if a.class <= Numeric then
				att = tup.schema.position(a)
			else 
				att = a
			end
			# we don't know the type yet...
			var = Variable.new(a,String,i,nil)
			if tup.schema.contains(var) then
				realVar = tup.schema.variable(a).clone
				realVar.position = i
				i = i + 1
				items << realVar
			end
		end
		#return projection
		

		newSchema = Schema.new(tup.schema.name.to_s+"-proj",items)

		#print "ORIGINAL TUPLE is #{tup}\n"
		#print "SCHEMA IS #{newSchema.inspect}\n"
		#print "PROJECTED TUPLE is #{projection.to_s}\n"

		projection.schema = newSchema
		return projection
	end
end
