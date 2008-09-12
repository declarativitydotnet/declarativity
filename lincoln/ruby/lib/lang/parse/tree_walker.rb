# a general Treetop walker that fires off callbacks registered by the caller.

class TreeWalker
	class Handler
		def initialize()	
			#@term = string
			#@func = func
			#@recurse = recurse
		end
		def semantic
			# not implemented
			raise "not implemented at base class"
			
		end
		def set_token(str)
			@token = str
		end
		def token
			return @token
		end
	end


	def initialize(tree)
		@tree = tree
		@hash = Hash.new
	end

	def add_handler(string,func,recurse)

		if @hash[string] then
			# print "hmm, already have a handler for this term: "+string+"\n"
		else
			func.set_token(string)
			@hash[string] = func
		end
	end

	def handlers
		@hash.each do |k|
			# puts k
			# puts @hash[k]
		end
	end

	def handle(elem)
		# this needs to be reversed: foreach method, probe hash.
		@hash.each do |k|
			meth = "elem."+k[0]
			stm = "defined? "+meth
			if (eval(stm)) then
				p = eval(meth)
				h = k[1]
				h.set_token(k[0])

				sem = (defined? p.text_value) ? p.text_value : p.to_s
				h.semantic(sem,p)
				if (@verbose == 'v') then  
					print k[0]+ ": ("+sem+")\n"
				end 
				#print "INTER: #{k[0]}\n"	
			end 
		end
	end

	def handle_fast(elem)
		elem.methods.each do |m|
			k = @hash[m.to_s]
			if !k.nil? then 
				#puts m.inspect
				p = eval("elem."+m.to_s)
				k.set_token(m.to_s)
				
				sem = (defined? p.text_value) ? p.text_value : p.to_s
				k.semantic(sem,p)
				if (@verbose == 'v') then  
					print m.to_s+ ": ("+sem+")\n"
				end 
				#print "INTER: #{m.to_s}\n"
				
				
			end
		end
	end
	def mywalk(node)
		if (defined? node.elements) then
			node.elements.each do |elem|
			# should be using handle_fast...
				ptr = handle(elem)
				##ptr = handle_fast(elem)
				# depth first...?
				if (elem.nonterminal?) then
					mywalk(elem)	
				end
			end
		end
	end
	def walk(verbose)
		@verbose = verbose
		mywalk(@tree)
	end

	
end




