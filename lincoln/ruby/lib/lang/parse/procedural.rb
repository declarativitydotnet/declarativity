def print_table(name,tuple)
        print name+"("+tuple.join(",")+");\n"
end


# had to.  will revisit.
def suck_nums(node)
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



