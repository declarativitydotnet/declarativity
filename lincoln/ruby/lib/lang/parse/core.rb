#@ this grammar is mostly auto-generated from the rats! grammar of the java implementation of P2.
# rats!, treetop and peg/leg are all implementations of packrat parsers, which are memoizing
# parsers for Parsing expression Grammars (PEGs).
# syntax-directed translation (rats! -> treetop) was performed using a PEG grammar in the peg/leg
# syntax.  the grammar recognizes a rats! grammar and outputs an equivalent treetop grammar.
#
# unfortunately, the evaluation strategy for treetop programs seems to differ significantly from
# that of rats!.  the auto-generated grammar contained numerous instances of left-recursive rules
# that caused stack overflows in ruby.  these all had to be manually re-written to remove
# left-recursion. (note: these rewrites need to be carefully reread: the operator precedence is
# likely thrown off)

# also, PEG rules with uppercase first letters causes namespace issues in ruby.  before I figured this
# out, I was unable to descend below a nonterminal semantic element, and structured the code in 
# some odd ways to work around this (see the semantic block for "name" (previously "Name"))


module Overlog
  include Treetop::Runtime

  def root
    @root || :pprogram
  end

  include Ddl

  module Pprogram0
    def pprogramname
      elements[0]
    end

    def Spacing
      elements[1]
    end

    def Clauses
      elements[2]
    end
  end

  def _nt_pprogram
    start_index = index
    if node_cache[:pprogram].has_key?(index)
      cached = node_cache[:pprogram][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_pprogramname
    s0 << r1
    if r1
      r2 = _nt_Spacing
      s0 << r2
      if r2
        r3 = _nt_Clauses
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Pprogram0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:pprogram][start_index] = r0

    return r0
  end

  module Pprogramname0
    def Spacing
      elements[1]
    end

    def word
      elements[2]
    end

  end

  module Pprogramname1
				def pprogramname
					return word.text_value
				end
  end

  def _nt_pprogramname
    start_index = index
    if node_cache[:pprogramname].has_key?(index)
      cached = node_cache[:pprogramname][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('program', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 7))
      @index += 7
    else
      terminal_parse_failure('program')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_Spacing
      s0 << r2
      if r2
        r3 = _nt_word
        s0 << r3
        if r3
          if input.index(';', index) == index
            r4 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure(';')
            r4 = nil
          end
          s0 << r4
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Pprogramname0)
      r0.extend(Pprogramname1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:pprogramname][start_index] = r0

    return r0
  end

  module Clauses0
    def Clause
      elements[0]
    end

    def semicolon
      elements[1]
    end
  end

  def _nt_Clauses
    start_index = index
    if node_cache[:Clauses].has_key?(index)
      cached = node_cache[:Clauses][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      i1, s1 = index, []
      r2 = _nt_Clause
      s1 << r2
      if r2
        r3 = _nt_semicolon
        s1 << r3
      end
      if s1.last
        r1 = (SyntaxNode).new(input, i1...index, s1)
        r1.extend(Clauses0)
      else
        self.index = i1
        r1 = nil
      end
      if r1
        s0 << r1
      else
        break
      end
    end
    r0 = SyntaxNode.new(input, i0...index, s0)

    node_cache[:Clauses][start_index] = r0

    return r0
  end

  module Semicolon0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_semicolon
    start_index = index
    if node_cache[:semicolon].has_key?(index)
      cached = node_cache[:semicolon][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      if input.index(';', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure(';')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Semicolon0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:semicolon][start_index] = r0

    return r0
  end

  def _nt_Clause
    start_index = index
    if node_cache[:Clause].has_key?(index)
      cached = node_cache[:Clause][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_Rule
    if r1
      r0 = r1
    else
      r2 = _nt_Watch
      if r2
        r0 = r2
      else
        r3 = _nt_Fact
        if r3
          r0 = r3
        else
          r4 = _nt_Definition
          if r4
            r0 = r4
          else
            r5 = _nt_Require
            if r5
              r0 = r5
            else
              r6 = _nt_statement
              if r6
                r0 = r6
              else
                self.index = i0
                r0 = nil
              end
            end
          end
        end
      end
    end

    node_cache[:Clause][start_index] = r0

    return r0
  end

  def _nt_statement
    start_index = index
    if node_cache[:statement].has_key?(index)
      cached = node_cache[:statement][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = super

    node_cache[:statement][start_index] = r0

    return r0
  end

  module Require0
    def Spacing
      elements[1]
    end

    def DoubleQuote
      elements[2]
    end

    def Filename
      elements[3]
    end

    def DoubleQuote
      elements[4]
    end
  end

  module Require1
		    def Require
		      return self.Filename
	      end
  end

  def _nt_Require
    start_index = index
    if node_cache[:Require].has_key?(index)
      cached = node_cache[:Require][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('require', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 7))
      @index += 7
    else
      terminal_parse_failure('require')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_Spacing
      s0 << r2
      if r2
        r3 = _nt_DoubleQuote
        s0 << r3
        if r3
          r4 = _nt_Filename
          s0 << r4
          if r4
            r5 = _nt_DoubleQuote
            s0 << r5
          end
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Require0)
      r0.extend(Require1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Require][start_index] = r0

    return r0
  end

  module Fact0
    def ptablename
      elements[0]
    end

    def opar
      elements[1]
    end

    def expressionList
      elements[2]
    end

    def cpar
      elements[3]
    end
  end

  module Fact1
				def Fact
					return self.ptablename.text_value
				end
  end

  def _nt_Fact
    start_index = index
    if node_cache[:Fact].has_key?(index)
      cached = node_cache[:Fact][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_ptablename
    s0 << r1
    if r1
      r2 = _nt_opar
      s0 << r2
      if r2
        r3 = _nt_expressionList
        s0 << r3
        if r3
          r4 = _nt_cpar
          s0 << r4
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Fact0)
      r0.extend(Fact1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Fact][start_index] = r0

    return r0
  end

  module Rule0
    def rname
      elements[1]
    end

    def deleter
      elements[2]
    end

    def rulehead
      elements[3]
    end

    def followsfrom
      elements[4]
    end

    def RuleBody
      elements[5]
    end
  end

  module Rule1
 
				def Rule
					return self
				end
  end

  def _nt_Rule
    start_index = index
    if node_cache[:Rule].has_key?(index)
      cached = node_cache[:Rule][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('public', index) == index
      r2 = (SyntaxNode).new(input, index...(index + 6))
      @index += 6
    else
      terminal_parse_failure('public')
      r2 = nil
    end
    if r2
      r1 = r2
    else
      r1 = SyntaxNode.new(input, index...index)
    end
    s0 << r1
    if r1
      r3 = _nt_rname
      s0 << r3
      if r3
        r4 = _nt_deleter
        s0 << r4
        if r4
          r5 = _nt_rulehead
          s0 << r5
          if r5
            r6 = _nt_followsfrom
            s0 << r6
            if r6
              r7 = _nt_RuleBody
              s0 << r7
            end
          end
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Rule0)
      r0.extend(Rule1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Rule][start_index] = r0

    return r0
  end

  module Rname0
    def name
      elements[0]
    end

  end

  def _nt_rname
    start_index = index
    if node_cache[:rname].has_key?(index)
      cached = node_cache[:rname][index]
      @index = cached.interval.end if cached
      return cached
    end

    i1, s1 = index, []
    r2 = _nt_name
    s1 << r2
    if r2
      i3 = index
      if input.index('(', index) == index
        r4 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('(')
        r4 = nil
      end
      if r4
        r3 = nil
      else
        self.index = i3
        r3 = SyntaxNode.new(input, index...index)
      end
      s1 << r3
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Rname0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r0 = SyntaxNode.new(input, index...index)
    end

    node_cache[:rname][start_index] = r0

    return r0
  end

  module Followsfrom0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_followsfrom
    start_index = index
    if node_cache[:followsfrom].has_key?(index)
      cached = node_cache[:followsfrom][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      if input.index(':-', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 2))
        @index += 2
      else
        terminal_parse_failure(':-')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Followsfrom0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:followsfrom][start_index] = r0

    return r0
  end

  module Deleter0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  module Deleter1
				def delete
					#if text_value.eql?("") then
					#	return false
					#else
					#	return true
					#end
					return !text_value.eql?("")
				end
  end

  def _nt_deleter
    start_index = index
    if node_cache[:deleter].has_key?(index)
      cached = node_cache[:deleter][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      if input.index('delete', index) == index
        r3 = (SyntaxNode).new(input, index...(index + 6))
        @index += 6
      else
        terminal_parse_failure('delete')
        r3 = nil
      end
      if r3
        r2 = r3
      else
        r2 = SyntaxNode.new(input, index...index)
      end
      s0 << r2
      if r2
        r4 = _nt_Spacing
        s0 << r4
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Deleter0)
      r0.extend(Deleter1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:deleter][start_index] = r0

    return r0
  end

  def _nt_rulehead
    start_index = index
    if node_cache[:rulehead].has_key?(index)
      cached = node_cache[:rulehead][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_predicate

    node_cache[:rulehead][start_index] = r0

    return r0
  end

  def _nt_RuleBody
    start_index = index
    if node_cache[:RuleBody].has_key?(index)
      cached = node_cache[:RuleBody][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_TermList

    node_cache[:RuleBody][start_index] = r0

    return r0
  end

  module TermList0
    def Comma
      elements[0]
    end

    def Term
      elements[1]
    end
  end

  module TermList1
    def Term
      elements[0]
    end

  end

  def _nt_TermList
    start_index = index
    if node_cache[:TermList].has_key?(index)
      cached = node_cache[:TermList][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Term
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        r4 = _nt_Comma
        s3 << r4
        if r4
          r5 = _nt_Term
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(TermList0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(TermList1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:TermList][start_index] = r0

    return r0
  end

  def _nt_Term
    start_index = index
    if node_cache[:Term].has_key?(index)
      cached = node_cache[:Term][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_TableFunction
    if r1
      r0 = r1
    else
      r2 = _nt_predicate
      if r2
        r0 = r2
      else
        r3 = _nt_Assignment
        if r3
          r0 = r3
        else
          r4 = _nt_Selection
          if r4
            r0 = r4
          else
            self.index = i0
            r0 = nil
          end
        end
      end
    end

    node_cache[:Term][start_index] = r0

    return r0
  end

  module Definition0
    def opar
      elements[1]
    end

    def ptablename
      elements[2]
    end

    def Comma
      elements[3]
    end

    def Keys
      elements[4]
    end

    def Comma
      elements[5]
    end

    def Schema
      elements[6]
    end

    def cpar
      elements[7]
    end
  end

  module Definition1
    def opar
      elements[1]
    end

    def ptablename
      elements[2]
    end

    def Comma
      elements[3]
    end

    def Schema
      elements[4]
    end

    def cpar
      elements[5]
    end
  end

  module Definition2
				def Definition
					return ptablename.text_value
				end
  end

  def _nt_Definition
    start_index = index
    if node_cache[:Definition].has_key?(index)
      cached = node_cache[:Definition][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    if input.index('define', index) == index
      r2 = (SyntaxNode).new(input, index...(index + 6))
      @index += 6
    else
      terminal_parse_failure('define')
      r2 = nil
    end
    s1 << r2
    if r2
      r3 = _nt_opar
      s1 << r3
      if r3
        r4 = _nt_ptablename
        s1 << r4
        if r4
          r5 = _nt_Comma
          s1 << r5
          if r5
            r6 = _nt_Keys
            s1 << r6
            if r6
              r7 = _nt_Comma
              s1 << r7
              if r7
                r8 = _nt_Schema
                s1 << r8
                if r8
                  r9 = _nt_cpar
                  s1 << r9
                end
              end
            end
          end
        end
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Definition0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
      r0.extend(Definition2)
    else
      i10, s10 = index, []
      if input.index('define', index) == index
        r11 = (SyntaxNode).new(input, index...(index + 6))
        @index += 6
      else
        terminal_parse_failure('define')
        r11 = nil
      end
      s10 << r11
      if r11
        r12 = _nt_opar
        s10 << r12
        if r12
          r13 = _nt_ptablename
          s10 << r13
          if r13
            r14 = _nt_Comma
            s10 << r14
            if r14
              r15 = _nt_Schema
              s10 << r15
              if r15
                r16 = _nt_cpar
                s10 << r16
              end
            end
          end
        end
      end
      if s10.last
        r10 = (SyntaxNode).new(input, i10...index, s10)
        r10.extend(Definition1)
      else
        self.index = i10
        r10 = nil
      end
      if r10
        r0 = r10
        r0.extend(Definition2)
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Definition][start_index] = r0

    return r0
  end

  module Opar0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_opar
    start_index = index
    if node_cache[:opar].has_key?(index)
      cached = node_cache[:opar][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      if input.index('(', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('(')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Opar0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:opar][start_index] = r0

    return r0
  end

  module Cpar0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_cpar
    start_index = index
    if node_cache[:cpar].has_key?(index)
      cached = node_cache[:cpar][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      if input.index(')', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure(')')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Cpar0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:cpar][start_index] = r0

    return r0
  end

  module Keys0
    def IntegerList
      elements[2]
    end

  end

  module Keys1
  end

  def _nt_Keys
    start_index = index
    if node_cache[:Keys].has_key?(index)
      cached = node_cache[:Keys][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    if input.index('keys', index) == index
      r2 = (SyntaxNode).new(input, index...(index + 4))
      @index += 4
    else
      terminal_parse_failure('keys')
      r2 = nil
    end
    s1 << r2
    if r2
      if input.index('(', index) == index
        r3 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('(')
        r3 = nil
      end
      s1 << r3
      if r3
        r4 = _nt_IntegerList
        s1 << r4
        if r4
          if input.index(')', index) == index
            r5 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure(')')
            r5 = nil
          end
          s1 << r5
        end
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Keys0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      i6, s6 = index, []
      if input.index('keys', index) == index
        r7 = (SyntaxNode).new(input, index...(index + 4))
        @index += 4
      else
        terminal_parse_failure('keys')
        r7 = nil
      end
      s6 << r7
      if r7
        if input.index('(', index) == index
          r8 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('(')
          r8 = nil
        end
        s6 << r8
        if r8
          if input.index(')', index) == index
            r9 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure(')')
            r9 = nil
          end
          s6 << r9
        end
      end
      if s6.last
        r6 = (SyntaxNode).new(input, i6...index, s6)
        r6.extend(Keys1)
      else
        self.index = i6
        r6 = nil
      end
      if r6
        r0 = r6
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Keys][start_index] = r0

    return r0
  end

  module Schema0
    def TypeDefList
      elements[1]
    end

  end

  def _nt_Schema
    start_index = index
    if node_cache[:Schema].has_key?(index)
      cached = node_cache[:Schema][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('{', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('{')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_TypeDefList
      s0 << r2
      if r2
        if input.index('}', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('}')
          r3 = nil
        end
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Schema0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Schema][start_index] = r0

    return r0
  end

  module TypeDefList0
    def Comma
      elements[0]
    end

    def Type
      elements[1]
    end
  end

  module TypeDefList1
    def Type
      elements[0]
    end

  end

  def _nt_TypeDefList
    start_index = index
    if node_cache[:TypeDefList].has_key?(index)
      cached = node_cache[:TypeDefList][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Type
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        r4 = _nt_Comma
        s3 << r4
        if r4
          r5 = _nt_Type
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(TypeDefList0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(TypeDefList1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:TypeDefList][start_index] = r0

    return r0
  end

  module Watch0
    def opar
      elements[1]
    end

    def ptablename
      elements[2]
    end

    def watchword
      elements[3]
    end

    def cpar
      elements[4]
    end
  end

  module Watch1
				def Watch
					return self
				end
  end

  def _nt_Watch
    start_index = index
    if node_cache[:Watch].has_key?(index)
      cached = node_cache[:Watch][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('watch', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 5))
      @index += 5
    else
      terminal_parse_failure('watch')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_opar
      s0 << r2
      if r2
        r3 = _nt_ptablename
        s0 << r3
        if r3
          r4 = _nt_watchword
          s0 << r4
          if r4
            r5 = _nt_cpar
            s0 << r5
          end
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Watch0)
      r0.extend(Watch1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Watch][start_index] = r0

    return r0
  end

  module Watchword0
    def Comma
      elements[0]
    end

    def watchFlow
      elements[1]
    end
  end

  module Watchword1
			def text
				return watchFlow.text_value
			end
  end

  def _nt_watchword
    start_index = index
    if node_cache[:watchword].has_key?(index)
      cached = node_cache[:watchword][index]
      @index = cached.interval.end if cached
      return cached
    end

    i1, s1 = index, []
    r2 = _nt_Comma
    s1 << r2
    if r2
      r3 = _nt_watchFlow
      s1 << r3
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Watchword0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r0 = SyntaxNode.new(input, index...index)
    end

    node_cache[:watchword][start_index] = r0

    return r0
  end

  def _nt_watchFlow
    start_index = index
    if node_cache[:watchFlow].has_key?(index)
      cached = node_cache[:watchFlow][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      if input.index(Regexp.new('[taeidrs]'), index) == index
        r1 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        r1 = nil
      end
      if r1
        s0 << r1
      else
        break
      end
    end
    if s0.empty?
      self.index = i0
      r0 = nil
    else
      r0 = SyntaxNode.new(input, i0...index, s0)
    end

    node_cache[:watchFlow][start_index] = r0

    return r0
  end

  module TableFunction0
    def ptablename
      elements[0]
    end

    def opar
      elements[1]
    end

    def predicate
      elements[2]
    end

    def cpar
      elements[3]
    end
  end

  module TableFunction1
		    def TableFunction
		      return self
	      end
  end

  def _nt_TableFunction
    start_index = index
    if node_cache[:TableFunction].has_key?(index)
      cached = node_cache[:TableFunction][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_ptablename
    s0 << r1
    if r1
      r2 = _nt_opar
      s0 << r2
      if r2
        r3 = _nt_predicate
        s0 << r3
        if r3
          r4 = _nt_cpar
          s0 << r4
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(TableFunction0)
      r0.extend(TableFunction1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:TableFunction][start_index] = r0

    return r0
  end

  module Predicate0
    def ptablename
      elements[1]
    end

    def eventModifier
      elements[2]
    end

    def arguments
      elements[3]
    end
  end

  module Predicate1
					def predicate
						#return self.ptablename
						return self
					end
					#def args
					#	return arguments.value
					#end
  end

  def _nt_predicate
    start_index = index
    if node_cache[:predicate].has_key?(index)
      cached = node_cache[:predicate][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r2 = _nt_notin
    if r2
      r1 = r2
    else
      r1 = SyntaxNode.new(input, index...index)
    end
    s0 << r1
    if r1
      r3 = _nt_ptablename
      s0 << r3
      if r3
        r4 = _nt_eventModifier
        s0 << r4
        if r4
          r5 = _nt_arguments
          s0 << r5
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Predicate0)
      r0.extend(Predicate1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:predicate][start_index] = r0

    return r0
  end

  module Notin0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  module Notin1
			def naught
				return !text_value.eql?("")
			end
  end

  def _nt_notin
    start_index = index
    if node_cache[:notin].has_key?(index)
      cached = node_cache[:notin][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      if input.index('notin', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 5))
        @index += 5
      else
        terminal_parse_failure('notin')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Notin0)
      r0.extend(Notin1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:notin][start_index] = r0

    return r0
  end

  module Assignment0
    def variable
      elements[0]
    end

    def Spacing
      elements[1]
    end

    def Spacing
      elements[3]
    end

    def expression
      elements[4]
    end
  end

  module Assignment1
				def Assignment
					return self
				end
  end

  def _nt_Assignment
    start_index = index
    if node_cache[:Assignment].has_key?(index)
      cached = node_cache[:Assignment][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_variable
    s0 << r1
    if r1
      r2 = _nt_Spacing
      s0 << r2
      if r2
        if input.index(':=', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 2))
          @index += 2
        else
          terminal_parse_failure(':=')
          r3 = nil
        end
        s0 << r3
        if r3
          r4 = _nt_Spacing
          s0 << r4
          if r4
            r5 = _nt_expression
            s0 << r5
          end
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Assignment0)
      r0.extend(Assignment1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Assignment][start_index] = r0

    return r0
  end

  module Selection0
				def Selection
					return self
				end
  end

  def _nt_Selection
    start_index = index
    if node_cache[:Selection].has_key?(index)
      cached = node_cache[:Selection][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_expression
    r0.extend(Selection0)

    node_cache[:Selection][start_index] = r0

    return r0
  end

  module Expression0
			def value 
				return text_value

			end
  end

  def _nt_expression
    start_index = index
    if node_cache[:expression].has_key?(index)
      cached = node_cache[:expression][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_IfElseexpression
    r0.extend(Expression0)

    node_cache[:expression][start_index] = r0

    return r0
  end

  module ExpressionList0
    def Comma
      elements[0]
    end

    def uExpression
      elements[1]
    end
  end

  module ExpressionList1
    def uExpression
      elements[0]
    end

  end

  module ExpressionList2
			def value 
				return 
			end
  end

  def _nt_expressionList
    start_index = index
    if node_cache[:expressionList].has_key?(index)
      cached = node_cache[:expressionList][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_uExpression
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        r4 = _nt_Comma
        s3 << r4
        if r4
          r5 = _nt_uExpression
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(ExpressionList0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(ExpressionList1)
      r0.extend(ExpressionList2)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:expressionList][start_index] = r0

    return r0
  end

  module UExpression0
			#@def uExpression
			#	return self
			#end
  end

  def _nt_uExpression
    start_index = index
    if node_cache[:uExpression].has_key?(index)
      cached = node_cache[:uExpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_expression
    r0.extend(UExpression0)

    node_cache[:uExpression][start_index] = r0

    return r0
  end

  module IfElseexpression0
    def LogicalOrexpression
      elements[0]
    end

    def expression
      elements[2]
    end

    def expression
      elements[4]
    end
  end

  def _nt_IfElseexpression
    start_index = index
    if node_cache[:IfElseexpression].has_key?(index)
      cached = node_cache[:IfElseexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_LogicalOrexpression
    s1 << r2
    if r2
      if input.index('?', index) == index
        r3 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('?')
        r3 = nil
      end
      s1 << r3
      if r3
        r4 = _nt_expression
        s1 << r4
        if r4
          if input.index(':', index) == index
            r5 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure(':')
            r5 = nil
          end
          s1 << r5
          if r5
            r6 = _nt_expression
            s1 << r6
          end
        end
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(IfElseexpression0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r7 = _nt_LogicalOrexpression
      if r7
        r0 = r7
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:IfElseexpression][start_index] = r0

    return r0
  end

  module LogicalOrexpression0
    def LogicalOrexpression
      elements[0]
    end

    def LogicalAndexpression
      elements[2]
    end
  end

  def _nt_LogicalOrexpression
    start_index = index
    if node_cache[:LogicalOrexpression].has_key?(index)
      cached = node_cache[:LogicalOrexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_LogicalAndexpression
    if r1
      r0 = r1
    else
      i2, s2 = index, []
      r3 = _nt_LogicalOrexpression
      s2 << r3
      if r3
        if input.index('||', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 2))
          @index += 2
        else
          terminal_parse_failure('||')
          r4 = nil
        end
        s2 << r4
        if r4
          r5 = _nt_LogicalAndexpression
          s2 << r5
        end
      end
      if s2.last
        r2 = (SyntaxNode).new(input, i2...index, s2)
        r2.extend(LogicalOrexpression0)
      else
        self.index = i2
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:LogicalOrexpression][start_index] = r0

    return r0
  end

  module LogicalAndexpression0
    def LogicalAndexpression
      elements[0]
    end

    def Equalityexpression
      elements[2]
    end
  end

  def _nt_LogicalAndexpression
    start_index = index
    if node_cache[:LogicalAndexpression].has_key?(index)
      cached = node_cache[:LogicalAndexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_Equalityexpression
    if r1
      r0 = r1
    else
      i2, s2 = index, []
      r3 = _nt_LogicalAndexpression
      s2 << r3
      if r3
        if input.index('&&', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 2))
          @index += 2
        else
          terminal_parse_failure('&&')
          r4 = nil
        end
        s2 << r4
        if r4
          r5 = _nt_Equalityexpression
          s2 << r5
        end
      end
      if s2.last
        r2 = (SyntaxNode).new(input, i2...index, s2)
        r2.extend(LogicalAndexpression0)
      else
        self.index = i2
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:LogicalAndexpression][start_index] = r0

    return r0
  end

  module Equalityexpression0
    def Inequalityexpression
      elements[0]
    end

    def eqop
      elements[1]
    end

    def Equalityexpression
      elements[2]
    end
  end

  def _nt_Equalityexpression
    start_index = index
    if node_cache[:Equalityexpression].has_key?(index)
      cached = node_cache[:Equalityexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_Inequalityexpression
    s1 << r2
    if r2
      r3 = _nt_eqop
      s1 << r3
      if r3
        r4 = _nt_Equalityexpression
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Equalityexpression0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r5 = _nt_Inequalityexpression
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Equalityexpression][start_index] = r0

    return r0
  end

  module Eqop0
    def Spacing
      elements[0]
    end

    def EqualityOperator
      elements[1]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_eqop
    start_index = index
    if node_cache[:eqop].has_key?(index)
      cached = node_cache[:eqop][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      r2 = _nt_EqualityOperator
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Eqop0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:eqop][start_index] = r0

    return r0
  end

  def _nt_EqualityOperator
    start_index = index
    if node_cache[:EqualityOperator].has_key?(index)
      cached = node_cache[:EqualityOperator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('==', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 2))
      @index += 2
    else
      terminal_parse_failure('==')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('!=', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 2))
        @index += 2
      else
        terminal_parse_failure('!=')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        if input.index('<>', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 2))
          @index += 2
        else
          terminal_parse_failure('<>')
          r3 = nil
        end
        if r3
          r0 = r3
        else
          self.index = i0
          r0 = nil
        end
      end
    end

    node_cache[:EqualityOperator][start_index] = r0

    return r0
  end

  module Inequalityexpression0
    def Shiftexpression
      elements[0]
    end

    def ineq
      elements[1]
    end

    def Inequalityexpression
      elements[2]
    end
  end

  def _nt_Inequalityexpression
    start_index = index
    if node_cache[:Inequalityexpression].has_key?(index)
      cached = node_cache[:Inequalityexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_Shiftexpression
    s1 << r2
    if r2
      r3 = _nt_ineq
      s1 << r3
      if r3
        r4 = _nt_Inequalityexpression
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Inequalityexpression0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r5 = _nt_Shiftexpression
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Inequalityexpression][start_index] = r0

    return r0
  end

  module Ineq0
    def Spacing
      elements[0]
    end

    def InequalityOperator
      elements[1]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_ineq
    start_index = index
    if node_cache[:ineq].has_key?(index)
      cached = node_cache[:ineq][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      r2 = _nt_InequalityOperator
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Ineq0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:ineq][start_index] = r0

    return r0
  end

  def _nt_InequalityOperator
    start_index = index
    if node_cache[:InequalityOperator].has_key?(index)
      cached = node_cache[:InequalityOperator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('<=', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 2))
      @index += 2
    else
      terminal_parse_failure('<=')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('>=', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 2))
        @index += 2
      else
        terminal_parse_failure('>=')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        if input.index('<', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('<')
          r3 = nil
        end
        if r3
          r0 = r3
        else
          if input.index('>', index) == index
            r4 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure('>')
            r4 = nil
          end
          if r4
            r0 = r4
          else
            self.index = i0
            r0 = nil
          end
        end
      end
    end

    node_cache[:InequalityOperator][start_index] = r0

    return r0
  end

  module Shiftexpression0
    def Shiftexpression
      elements[0]
    end

    def ShiftOperator
      elements[1]
    end

    def Additiveexpression
      elements[2]
    end
  end

  def _nt_Shiftexpression
    start_index = index
    if node_cache[:Shiftexpression].has_key?(index)
      cached = node_cache[:Shiftexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_Additiveexpression
    if r1
      r0 = r1
    else
      i2, s2 = index, []
      r3 = _nt_Shiftexpression
      s2 << r3
      if r3
        r4 = _nt_ShiftOperator
        s2 << r4
        if r4
          r5 = _nt_Additiveexpression
          s2 << r5
        end
      end
      if s2.last
        r2 = (SyntaxNode).new(input, i2...index, s2)
        r2.extend(Shiftexpression0)
      else
        self.index = i2
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Shiftexpression][start_index] = r0

    return r0
  end

  def _nt_ShiftOperator
    start_index = index
    if node_cache[:ShiftOperator].has_key?(index)
      cached = node_cache[:ShiftOperator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('<<', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 2))
      @index += 2
    else
      terminal_parse_failure('<<')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('>>', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 2))
        @index += 2
      else
        terminal_parse_failure('>>')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:ShiftOperator][start_index] = r0

    return r0
  end

  module Additiveexpression0
    def Multiplicativeexpression
      elements[0]
    end

    def addop
      elements[1]
    end

    def Additiveexpression
      elements[2]
    end
  end

  def _nt_Additiveexpression
    start_index = index
    if node_cache[:Additiveexpression].has_key?(index)
      cached = node_cache[:Additiveexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_Multiplicativeexpression
    s1 << r2
    if r2
      r3 = _nt_addop
      s1 << r3
      if r3
        r4 = _nt_Additiveexpression
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Additiveexpression0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r5 = _nt_Multiplicativeexpression
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Additiveexpression][start_index] = r0

    return r0
  end

  module AdditiveOperator0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_AdditiveOperator
    start_index = index
    if node_cache[:AdditiveOperator].has_key?(index)
      cached = node_cache[:AdditiveOperator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_Spacing
    s1 << r2
    if r2
      if input.index('+', index) == index
        r3 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('+')
        r3 = nil
      end
      s1 << r3
      if r3
        r4 = _nt_Spacing
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(AdditiveOperator0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('-', index) == index
        r5 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('-')
        r5 = nil
      end
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:AdditiveOperator][start_index] = r0

    return r0
  end

  module Multiplicativeexpression0
    def Unaryexpression
      elements[0]
    end

    def multop
      elements[1]
    end

    def Multiplicativeexpression
      elements[2]
    end
  end

  def _nt_Multiplicativeexpression
    start_index = index
    if node_cache[:Multiplicativeexpression].has_key?(index)
      cached = node_cache[:Multiplicativeexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_Unaryexpression
    s1 << r2
    if r2
      r3 = _nt_multop
      s1 << r3
      if r3
        r4 = _nt_Multiplicativeexpression
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Multiplicativeexpression0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r5 = _nt_Unaryexpression
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Multiplicativeexpression][start_index] = r0

    return r0
  end

  module Multop0
    def Spacing
      elements[0]
    end

    def MultiplicativeOperator
      elements[1]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_multop
    start_index = index
    if node_cache[:multop].has_key?(index)
      cached = node_cache[:multop][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      r2 = _nt_MultiplicativeOperator
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Multop0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:multop][start_index] = r0

    return r0
  end

  module Addop0
    def Spacing
      elements[0]
    end

    def AdditiveOperator
      elements[1]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_addop
    start_index = index
    if node_cache[:addop].has_key?(index)
      cached = node_cache[:addop][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      r2 = _nt_AdditiveOperator
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Addop0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:addop][start_index] = r0

    return r0
  end

  def _nt_MultiplicativeOperator
    start_index = index
    if node_cache[:MultiplicativeOperator].has_key?(index)
      cached = node_cache[:MultiplicativeOperator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('*', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('*')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('/', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('/')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        if input.index('%', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('%')
          r3 = nil
        end
        if r3
          r0 = r3
        else
          self.index = i0
          r0 = nil
        end
      end
    end

    node_cache[:MultiplicativeOperator][start_index] = r0

    return r0
  end

  def _nt_Unaryexpression
    start_index = index
    if node_cache[:Unaryexpression].has_key?(index)
      cached = node_cache[:Unaryexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_Postfixexpression
    if r1
      r0 = r1
    else
      r2 = _nt_Castexpression
      if r2
        r0 = r2
      else
        r3 = _nt_LogicalNegationexpression
        if r3
          r0 = r3
        else
          r4 = _nt_Inclusiveexpression
          if r4
            r0 = r4
          else
            self.index = i0
            r0 = nil
          end
        end
      end
    end

    node_cache[:Unaryexpression][start_index] = r0

    return r0
  end

  module LogicalNegationexpression0
    def Unaryexpression
      elements[1]
    end
  end

  def _nt_LogicalNegationexpression
    start_index = index
    if node_cache[:LogicalNegationexpression].has_key?(index)
      cached = node_cache[:LogicalNegationexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('!', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('!')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_Unaryexpression
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(LogicalNegationexpression0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:LogicalNegationexpression][start_index] = r0

    return r0
  end

  module Castexpression0
    def Type
      elements[1]
    end

    def Unaryexpression
      elements[3]
    end
  end

  def _nt_Castexpression
    start_index = index
    if node_cache[:Castexpression].has_key?(index)
      cached = node_cache[:Castexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('(', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('(')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_Type
      s0 << r2
      if r2
        if input.index(')', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure(')')
          r3 = nil
        end
        s0 << r3
        if r3
          r4 = _nt_Unaryexpression
          s0 << r4
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Castexpression0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Castexpression][start_index] = r0

    return r0
  end

  module Inclusiveexpression0
    def primaryexpression
      elements[0]
    end

    def Rangeexpression
      elements[2]
    end
  end

  module Inclusiveexpression1
    def primaryexpression
      elements[0]
    end

    def variable
      elements[2]
    end
  end

  def _nt_Inclusiveexpression
    start_index = index
    if node_cache[:Inclusiveexpression].has_key?(index)
      cached = node_cache[:Inclusiveexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_primaryexpression
    s1 << r2
    if r2
      if input.index('in', index) == index
        r3 = (SyntaxNode).new(input, index...(index + 2))
        @index += 2
      else
        terminal_parse_failure('in')
        r3 = nil
      end
      s1 << r3
      if r3
        r4 = _nt_Rangeexpression
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Inclusiveexpression0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      i5, s5 = index, []
      r6 = _nt_primaryexpression
      s5 << r6
      if r6
        if input.index('in', index) == index
          r7 = (SyntaxNode).new(input, index...(index + 2))
          @index += 2
        else
          terminal_parse_failure('in')
          r7 = nil
        end
        s5 << r7
        if r7
          r8 = _nt_variable
          s5 << r8
        end
      end
      if s5.last
        r5 = (SyntaxNode).new(input, i5...index, s5)
        r5.extend(Inclusiveexpression1)
      else
        self.index = i5
        r5 = nil
      end
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Inclusiveexpression][start_index] = r0

    return r0
  end

  module Rangeexpression0
    def LeftRangeOperator
      elements[0]
    end

    def expression
      elements[1]
    end

    def expression
      elements[3]
    end

    def RightRangeOperator
      elements[4]
    end
  end

  def _nt_Rangeexpression
    start_index = index
    if node_cache[:Rangeexpression].has_key?(index)
      cached = node_cache[:Rangeexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_LeftRangeOperator
    s0 << r1
    if r1
      r2 = _nt_expression
      s0 << r2
      if r2
        if input.index(',', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure(',')
          r3 = nil
        end
        s0 << r3
        if r3
          r4 = _nt_expression
          s0 << r4
          if r4
            r5 = _nt_RightRangeOperator
            s0 << r5
          end
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Rangeexpression0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Rangeexpression][start_index] = r0

    return r0
  end

  def _nt_LeftRangeOperator
    start_index = index
    if node_cache[:LeftRangeOperator].has_key?(index)
      cached = node_cache[:LeftRangeOperator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('[', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('[')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('(', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('(')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:LeftRangeOperator][start_index] = r0

    return r0
  end

  def _nt_RightRangeOperator
    start_index = index
    if node_cache[:RightRangeOperator].has_key?(index)
      cached = node_cache[:RightRangeOperator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index(']', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure(']')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index(')', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure(')')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:RightRangeOperator][start_index] = r0

    return r0
  end

  module Postfixexpression0
    def Postfixexpression
      elements[0]
    end

    def arguments
      elements[1]
    end
  end

  module Postfixexpression1
    def Postfixexpression
      elements[0]
    end

    def Constant
      elements[2]
    end

    def ArrayIndex
      elements[4]
    end
  end

  module Postfixexpression2
    def Postfixexpression
      elements[0]
    end

    def Increment
      elements[2]
    end
  end

  module Postfixexpression3
    def Postfixexpression
      elements[0]
    end

    def Decrement
      elements[2]
    end
  end

  def _nt_Postfixexpression
    start_index = index
    if node_cache[:Postfixexpression].has_key?(index)
      cached = node_cache[:Postfixexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_methodcall
    if r1
      r0 = r1
    else
      r2 = _nt_primaryexpression
      if r2
        r0 = r2
      else
        i3, s3 = index, []
        r4 = _nt_Postfixexpression
        s3 << r4
        if r4
          r5 = _nt_arguments
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(Postfixexpression0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          r0 = r3
        else
          i6, s6 = index, []
          r7 = _nt_Postfixexpression
          s6 << r7
          if r7
            if input.index('[', index) == index
              r8 = (SyntaxNode).new(input, index...(index + 1))
              @index += 1
            else
              terminal_parse_failure('[')
              r8 = nil
            end
            s6 << r8
            if r8
              r9 = _nt_Constant
              s6 << r9
              if r9
                if input.index(']', index) == index
                  r10 = (SyntaxNode).new(input, index...(index + 1))
                  @index += 1
                else
                  terminal_parse_failure(']')
                  r10 = nil
                end
                s6 << r10
                if r10
                  r11 = _nt_ArrayIndex
                  s6 << r11
                end
              end
            end
          end
          if s6.last
            r6 = (SyntaxNode).new(input, i6...index, s6)
            r6.extend(Postfixexpression1)
          else
            self.index = i6
            r6 = nil
          end
          if r6
            r0 = r6
          else
            i12, s12 = index, []
            r13 = _nt_Postfixexpression
            s12 << r13
            if r13
              if input.index('++', index) == index
                r14 = (SyntaxNode).new(input, index...(index + 2))
                @index += 2
              else
                terminal_parse_failure('++')
                r14 = nil
              end
              s12 << r14
              if r14
                r15 = _nt_Increment
                s12 << r15
              end
            end
            if s12.last
              r12 = (SyntaxNode).new(input, i12...index, s12)
              r12.extend(Postfixexpression2)
            else
              self.index = i12
              r12 = nil
            end
            if r12
              r0 = r12
            else
              i16, s16 = index, []
              r17 = _nt_Postfixexpression
              s16 << r17
              if r17
                if input.index('--', index) == index
                  r18 = (SyntaxNode).new(input, index...(index + 2))
                  @index += 2
                else
                  terminal_parse_failure('--')
                  r18 = nil
                end
                s16 << r18
                if r18
                  r19 = _nt_Decrement
                  s16 << r19
                end
              end
              if s16.last
                r16 = (SyntaxNode).new(input, i16...index, s16)
                r16.extend(Postfixexpression3)
              else
                self.index = i16
                r16 = nil
              end
              if r16
                r0 = r16
              else
                self.index = i0
                r0 = nil
              end
            end
          end
        end
      end
    end

    node_cache[:Postfixexpression][start_index] = r0

    return r0
  end

  module Methodcall0
    def primaryexpression
      elements[0]
    end

    def name
      elements[2]
    end

  end

  def _nt_methodcall
    start_index = index
    if node_cache[:methodcall].has_key?(index)
      cached = node_cache[:methodcall][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_primaryexpression
    s0 << r1
    if r1
      if input.index('.', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('.')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_name
        s0 << r3
        if r3
          r5 = _nt_arguments
          if r5
            r4 = r5
          else
            r4 = SyntaxNode.new(input, index...index)
          end
          s0 << r4
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Methodcall0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:methodcall][start_index] = r0

    return r0
  end

  module Primaryexpression0
    def Location
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Primaryexpression1
    def Aggregate
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Primaryexpression2
    def opar
      elements[0]
    end

    def expression
      elements[1]
    end

    def cpar
      elements[2]
    end
  end

  module Primaryexpression3
    def Constant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Primaryexpression4
    def Alias
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Primaryexpression5
    def variable
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Primaryexpression6
    def NewClass
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Primaryexpression7
    def Referencename
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Primaryexpression8
					#def primaryexpression
					#	return self
					#end
  end

  def _nt_primaryexpression
    start_index = index
    if node_cache[:primaryexpression].has_key?(index)
      cached = node_cache[:primaryexpression][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_Location
    s1 << r2
    if r2
      r3 = _nt_Spacing
      s1 << r3
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Primaryexpression0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
      r0.extend(Primaryexpression8)
    else
      i4, s4 = index, []
      r5 = _nt_Aggregate
      s4 << r5
      if r5
        r6 = _nt_Spacing
        s4 << r6
      end
      if s4.last
        r4 = (SyntaxNode).new(input, i4...index, s4)
        r4.extend(Primaryexpression1)
      else
        self.index = i4
        r4 = nil
      end
      if r4
        r0 = r4
        r0.extend(Primaryexpression8)
      else
        i7, s7 = index, []
        r8 = _nt_opar
        s7 << r8
        if r8
          r9 = _nt_expression
          s7 << r9
          if r9
            r10 = _nt_cpar
            s7 << r10
          end
        end
        if s7.last
          r7 = (SyntaxNode).new(input, i7...index, s7)
          r7.extend(Primaryexpression2)
        else
          self.index = i7
          r7 = nil
        end
        if r7
          r0 = r7
          r0.extend(Primaryexpression8)
        else
          i11, s11 = index, []
          r12 = _nt_Constant
          s11 << r12
          if r12
            r13 = _nt_Spacing
            s11 << r13
          end
          if s11.last
            r11 = (SyntaxNode).new(input, i11...index, s11)
            r11.extend(Primaryexpression3)
          else
            self.index = i11
            r11 = nil
          end
          if r11
            r0 = r11
            r0.extend(Primaryexpression8)
          else
            i14, s14 = index, []
            r15 = _nt_Alias
            s14 << r15
            if r15
              r16 = _nt_Spacing
              s14 << r16
            end
            if s14.last
              r14 = (SyntaxNode).new(input, i14...index, s14)
              r14.extend(Primaryexpression4)
            else
              self.index = i14
              r14 = nil
            end
            if r14
              r0 = r14
              r0.extend(Primaryexpression8)
            else
              i17, s17 = index, []
              r18 = _nt_variable
              s17 << r18
              if r18
                r19 = _nt_Spacing
                s17 << r19
              end
              if s17.last
                r17 = (SyntaxNode).new(input, i17...index, s17)
                r17.extend(Primaryexpression5)
              else
                self.index = i17
                r17 = nil
              end
              if r17
                r0 = r17
                r0.extend(Primaryexpression8)
              else
                i20, s20 = index, []
                r21 = _nt_NewClass
                s20 << r21
                if r21
                  r22 = _nt_Spacing
                  s20 << r22
                end
                if s20.last
                  r20 = (SyntaxNode).new(input, i20...index, s20)
                  r20.extend(Primaryexpression6)
                else
                  self.index = i20
                  r20 = nil
                end
                if r20
                  r0 = r20
                  r0.extend(Primaryexpression8)
                else
                  i23, s23 = index, []
                  r24 = _nt_Referencename
                  s23 << r24
                  if r24
                    r25 = _nt_Spacing
                    s23 << r25
                  end
                  if s23.last
                    r23 = (SyntaxNode).new(input, i23...index, s23)
                    r23.extend(Primaryexpression7)
                  else
                    self.index = i23
                    r23 = nil
                  end
                  if r23
                    r0 = r23
                    r0.extend(Primaryexpression8)
                  else
                    self.index = i0
                    r0 = nil
                  end
                end
              end
            end
          end
        end
      end
    end

    node_cache[:primaryexpression][start_index] = r0

    return r0
  end

  module Arguments0
    def opar
      elements[0]
    end

    def cpar
      elements[2]
    end
  end

  module Arguments1
    def opar
      elements[0]
    end

    def cpar
      elements[1]
    end
  end

  module Arguments2
				#def arguments 
				#	return self
				#end
				#def value 	
				#	return expressionList.value
				#end
				def args
					return expressionList
				end
  end

  def _nt_arguments
    start_index = index
    if node_cache[:arguments].has_key?(index)
      cached = node_cache[:arguments][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_opar
    s1 << r2
    if r2
      r3 = _nt_expressionList
      s1 << r3
      if r3
        r4 = _nt_cpar
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Arguments0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      i5, s5 = index, []
      r6 = _nt_opar
      s5 << r6
      if r6
        r7 = _nt_cpar
        s5 << r7
      end
      if s5.last
        r5 = (SyntaxNode).new(input, i5...index, s5)
        r5.extend(Arguments1)
        r5.extend(Arguments2)
      else
        self.index = i5
        r5 = nil
      end
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:arguments][start_index] = r0

    return r0
  end

  module NewClass0
    def Typename
      elements[0]
    end

  end

  def _nt_NewClass
    start_index = index
    if node_cache[:NewClass].has_key?(index)
      cached = node_cache[:NewClass][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Typename
    s0 << r1
    if r1
      if input.index('.new', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 4))
        @index += 4
      else
        terminal_parse_failure('.new')
        r2 = nil
      end
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(NewClass0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:NewClass][start_index] = r0

    return r0
  end

  def _nt_Referencename
    start_index = index
    if node_cache[:Referencename].has_key?(index)
      cached = node_cache[:Referencename][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_name

    node_cache[:Referencename][start_index] = r0

    return r0
  end

  module Alias0
    def variable
      elements[0]
    end

    def IntegerConstant
      elements[3]
    end
  end

  def _nt_Alias
    start_index = index
    if node_cache[:Alias].has_key?(index)
      cached = node_cache[:Alias][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_variable
    s0 << r1
    if r1
      if input.index(':=', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 2))
        @index += 2
      else
        terminal_parse_failure(':=')
        r2 = nil
      end
      s0 << r2
      if r2
        if input.index('$', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('$')
          r3 = nil
        end
        s0 << r3
        if r3
          r4 = _nt_IntegerConstant
          s0 << r4
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Alias0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Alias][start_index] = r0

    return r0
  end

  def _nt_name
    start_index = index
    if node_cache[:name].has_key?(index)
      cached = node_cache[:name][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_word

    node_cache[:name][start_index] = r0

    return r0
  end

  module Word0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  module Word1
				def word
					return self
				end 
				def value 
					return text_value
				end
  end

  def _nt_word
    start_index = index
    if node_cache[:word].has_key?(index)
      cached = node_cache[:word][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      r2 = _nt_wordCharacters
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Word0)
      r0.extend(Word1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:word][start_index] = r0

    return r0
  end

  def _nt_wordCharacters
    start_index = index
    if node_cache[:wordCharacters].has_key?(index)
      cached = node_cache[:wordCharacters][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      if input.index(Regexp.new('[a-zA-Z0-9_]'), index) == index
        r1 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        r1 = nil
      end
      if r1
        s0 << r1
      else
        break
      end
    end
    r0 = SyntaxNode.new(input, i0...index, s0)

    node_cache[:wordCharacters][start_index] = r0

    return r0
  end

  def _nt_Spacing
    start_index = index
    if node_cache[:Spacing].has_key?(index)
      cached = node_cache[:Spacing][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      i1 = index
      r2 = _nt_Space
      if r2
        r1 = r2
      else
        r3 = _nt_LineTerminator
        if r3
          r1 = r3
        else
          r4 = _nt_TraditionalComment
          if r4
            r1 = r4
          else
            r5 = _nt_EndOfLineComment
            if r5
              r1 = r5
            else
              self.index = i1
              r1 = nil
            end
          end
        end
      end
      if r1
        s0 << r1
      else
        break
      end
    end
    r0 = SyntaxNode.new(input, i0...index, s0)

    node_cache[:Spacing][start_index] = r0

    return r0
  end

  def _nt_Space
    start_index = index
    if node_cache[:Space].has_key?(index)
      cached = node_cache[:Space][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[ \\t\\f]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:Space][start_index] = r0

    return r0
  end

  def _nt_DoubleQuote
    start_index = index
    if node_cache[:DoubleQuote].has_key?(index)
      cached = node_cache[:DoubleQuote][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index('"', index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('"')
      r0 = nil
    end

    node_cache[:DoubleQuote][start_index] = r0

    return r0
  end

  module TraditionalComment0
  end

  module TraditionalComment1
  end

  module TraditionalComment2
  end

  def _nt_TraditionalComment
    start_index = index
    if node_cache[:TraditionalComment].has_key?(index)
      cached = node_cache[:TraditionalComment][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('/*', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 2))
      @index += 2
    else
      terminal_parse_failure('/*')
      r1 = nil
    end
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3 = index
        i4, s4 = index, []
        if input.index('*', index) == index
          r5 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('*')
          r5 = nil
        end
        s4 << r5
        if r5
          i6 = index
          if input.index('/', index) == index
            r7 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure('/')
            r7 = nil
          end
          if r7
            r6 = nil
          else
            self.index = i6
            r6 = SyntaxNode.new(input, index...index)
          end
          s4 << r6
        end
        if s4.last
          r4 = (SyntaxNode).new(input, i4...index, s4)
          r4.extend(TraditionalComment0)
        else
          self.index = i4
          r4 = nil
        end
        if r4
          r3 = r4
        else
          i8, s8 = index, []
          i9 = index
          if input.index('*', index) == index
            r10 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure('*')
            r10 = nil
          end
          if r10
            r9 = nil
          else
            self.index = i9
            r9 = SyntaxNode.new(input, index...index)
          end
          s8 << r9
          if r9
            if index < input_length
              r11 = (SyntaxNode).new(input, index...(index + 1))
              @index += 1
            else
              terminal_parse_failure("any character")
              r11 = nil
            end
            s8 << r11
          end
          if s8.last
            r8 = (SyntaxNode).new(input, i8...index, s8)
            r8.extend(TraditionalComment1)
          else
            self.index = i8
            r8 = nil
          end
          if r8
            r3 = r8
          else
            self.index = i3
            r3 = nil
          end
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
      if r2
        if input.index('*/', index) == index
          r12 = (SyntaxNode).new(input, index...(index + 2))
          @index += 2
        else
          terminal_parse_failure('*/')
          r12 = nil
        end
        s0 << r12
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(TraditionalComment2)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:TraditionalComment][start_index] = r0

    return r0
  end

  module EndOfLineComment0
  end

  module EndOfLineComment1
  end

  def _nt_EndOfLineComment
    start_index = index
    if node_cache[:EndOfLineComment].has_key?(index)
      cached = node_cache[:EndOfLineComment][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('//', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 2))
      @index += 2
    else
      terminal_parse_failure('//')
      r1 = nil
    end
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        i4 = index
        if input.index(Regexp.new('[\\n\\r]'), index) == index
          r5 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          r5 = nil
        end
        if r5
          r4 = nil
        else
          self.index = i4
          r4 = SyntaxNode.new(input, index...index)
        end
        s3 << r4
        if r4
          if index < input_length
            r6 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure("any character")
            r6 = nil
          end
          s3 << r6
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(EndOfLineComment0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
      if r2
        r7 = _nt_LineTerminator
        s0 << r7
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(EndOfLineComment1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:EndOfLineComment][start_index] = r0

    return r0
  end

  module LineTerminator0
				def LineTerminator
					#@@lines = @@lines + 1
					return self
				end
  end

  def _nt_LineTerminator
    start_index = index
    if node_cache[:LineTerminator].has_key?(index)
      cached = node_cache[:LineTerminator][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('\r\n', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 4))
      @index += 4
    else
      terminal_parse_failure('\r\n')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r2 = _nt_Newline
      r2.extend(LineTerminator0)
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:LineTerminator][start_index] = r0

    return r0
  end

  def _nt_Newline
    start_index = index
    if node_cache[:Newline].has_key?(index)
      cached = node_cache[:Newline][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[\\r\\n]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:Newline][start_index] = r0

    return r0
  end

  def _nt_EndOfFile
    start_index = index
    if node_cache[:EndOfFile].has_key?(index)
      cached = node_cache[:EndOfFile][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if index < input_length
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure("any character")
      r1 = nil
    end
    if r1
      r0 = nil
    else
      self.index = i0
      r0 = SyntaxNode.new(input, index...index)
    end

    node_cache[:EndOfFile][start_index] = r0

    return r0
  end

  module Comma0
    def Spacing
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_Comma
    start_index = index
    if node_cache[:Comma].has_key?(index)
      cached = node_cache[:Comma][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Spacing
    s0 << r1
    if r1
      if input.index(",", index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure(",")
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_Spacing
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Comma0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Comma][start_index] = r0

    return r0
  end

  def _nt_name
    start_index = index
    if node_cache[:name].has_key?(index)
      cached = node_cache[:name][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_word

    node_cache[:name][start_index] = r0

    return r0
  end

  def _nt_word
    start_index = index
    if node_cache[:word].has_key?(index)
      cached = node_cache[:word][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_wordCharacters

    node_cache[:word][start_index] = r0

    return r0
  end

  module EventModifier0
    def eventType
      elements[1]
    end
  end

  module EventModifier1
		    def eventModifier
		      return self.eventType
	      end
  end

  def _nt_eventModifier
    start_index = index
    if node_cache[:eventModifier].has_key?(index)
      cached = node_cache[:eventModifier][index]
      @index = cached.interval.end if cached
      return cached
    end

    i1, s1 = index, []
    if input.index('#', index) == index
      r2 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('#')
      r2 = nil
    end
    s1 << r2
    if r2
      r3 = _nt_eventType
      s1 << r3
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(EventModifier0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r0 = SyntaxNode.new(input, index...index)
    end

    node_cache[:eventModifier][start_index] = r0

    return r0
  end

  def _nt_eventType
    start_index = index
    if node_cache[:eventType].has_key?(index)
      cached = node_cache[:eventType][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('insert', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 6))
      @index += 6
    else
      terminal_parse_failure('insert')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('delete', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 6))
        @index += 6
      else
        terminal_parse_failure('delete')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:eventType][start_index] = r0

    return r0
  end

  module Ptablename0
    def scope
      elements[0]
    end

    def name
      elements[2]
    end
  end

  def _nt_ptablename
    start_index = index
    if node_cache[:ptablename].has_key?(index)
      cached = node_cache[:ptablename][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_scope
    s1 << r2
    if r2
      if input.index('::', index) == index
        r3 = (SyntaxNode).new(input, index...(index + 2))
        @index += 2
      else
        terminal_parse_failure('::')
        r3 = nil
      end
      s1 << r3
      if r3
        r4 = _nt_name
        s1 << r4
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Ptablename0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      r5 = _nt_name
      if r5
        r0 = r5
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:ptablename][start_index] = r0

    return r0
  end

  def _nt_scope
    start_index = index
    if node_cache[:scope].has_key?(index)
      cached = node_cache[:scope][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_name

    node_cache[:scope][start_index] = r0

    return r0
  end

  def _nt_variable
    start_index = index
    if node_cache[:variable].has_key?(index)
      cached = node_cache[:variable][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_name
    if r1
      r0 = r1
    else
      if input.index('_', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('_')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        if input.index('*', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('*')
          r3 = nil
        end
        if r3
          r0 = r3
        else
          self.index = i0
          r0 = nil
        end
      end
    end

    node_cache[:variable][start_index] = r0

    return r0
  end

  module Location0
    def variable
      elements[1]
    end
  end

  def _nt_Location
    start_index = index
    if node_cache[:Location].has_key?(index)
      cached = node_cache[:Location][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('@', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('@')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_variable
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Location0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Location][start_index] = r0

    return r0
  end

  module Aggregate0
    def name
      elements[0]
    end

    def aggregatevariable
      elements[2]
    end

  end

  module Aggregate1
			def func
				return name
			end
  end

  def _nt_Aggregate
    start_index = index
    if node_cache[:Aggregate].has_key?(index)
      cached = node_cache[:Aggregate][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_name
    s0 << r1
    if r1
      if input.index('<', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('<')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_aggregatevariable
        s0 << r3
        if r3
          if input.index('>', index) == index
            r4 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure('>')
            r4 = nil
          end
          s0 << r4
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Aggregate0)
      r0.extend(Aggregate1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Aggregate][start_index] = r0

    return r0
  end

  def _nt_aggregatevariable
    start_index = index
    if node_cache[:aggregatevariable].has_key?(index)
      cached = node_cache[:aggregatevariable][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_Location
    if r1
      r0 = r1
    else
      r2 = _nt_variable
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:aggregatevariable][start_index] = r0

    return r0
  end

  module AttributePosition0
    def Spacing
      elements[2]
    end
  end

  def _nt_AttributePosition
    start_index = index
    if node_cache[:AttributePosition].has_key?(index)
      cached = node_cache[:AttributePosition][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('$', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('$')
      r1 = nil
    end
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        if input.index(Regexp.new('[0-9]'), index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      if s2.empty?
        self.index = i2
        r2 = nil
      else
        r2 = SyntaxNode.new(input, i2...index, s2)
      end
      s0 << r2
      if r2
        r4 = _nt_Spacing
        s0 << r4
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(AttributePosition0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:AttributePosition][start_index] = r0

    return r0
  end

  module Type0
    def Typename
      elements[0]
    end

    def Spacing
      elements[2]
    end
  end

  def _nt_Type
    start_index = index
    if node_cache[:Type].has_key?(index)
      cached = node_cache[:Type][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Typename
    s0 << r1
    if r1
      r3 = _nt_Dimensions
      if r3
        r2 = r3
      else
        r2 = SyntaxNode.new(input, index...index)
      end
      s0 << r2
      if r2
        r4 = _nt_Spacing
        s0 << r4
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Type0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Type][start_index] = r0

    return r0
  end

  def _nt_Typename
    start_index = index
    if node_cache[:Typename].has_key?(index)
      cached = node_cache[:Typename][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_PrimitiveType
    if r1
      r0 = r1
    else
      r2 = _nt_ClassType
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Typename][start_index] = r0

    return r0
  end

  module Filename0
    def name
      elements[1]
    end
  end

  module Filename1
    def name
      elements[0]
    end

  end

  def _nt_Filename
    start_index = index
    if node_cache[:Filename].has_key?(index)
      cached = node_cache[:Filename][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_name
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        if input.index('/', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('/')
          r4 = nil
        end
        s3 << r4
        if r4
          r5 = _nt_name
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(Filename0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Filename1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Filename][start_index] = r0

    return r0
  end

  def _nt_PrimitiveType
    start_index = index
    if node_cache[:PrimitiveType].has_key?(index)
      cached = node_cache[:PrimitiveType][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('byte', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 4))
      @index += 4
    else
      terminal_parse_failure('byte')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('short', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 5))
        @index += 5
      else
        terminal_parse_failure('short')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        if input.index('char', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 4))
          @index += 4
        else
          terminal_parse_failure('char')
          r3 = nil
        end
        if r3
          r0 = r3
        else
          if input.index('string', index) == index
            r4 = (SyntaxNode).new(input, index...(index + 6))
            @index += 6
          else
            terminal_parse_failure('string')
            r4 = nil
          end
          if r4
            r0 = r4
          else
            if input.index('int', index) == index
              r5 = (SyntaxNode).new(input, index...(index + 3))
              @index += 3
            else
              terminal_parse_failure('int')
              r5 = nil
            end
            if r5
              r0 = r5
            else
              if input.index('long', index) == index
                r6 = (SyntaxNode).new(input, index...(index + 4))
                @index += 4
              else
                terminal_parse_failure('long')
                r6 = nil
              end
              if r6
                r0 = r6
              else
                if input.index('float', index) == index
                  r7 = (SyntaxNode).new(input, index...(index + 5))
                  @index += 5
                else
                  terminal_parse_failure('float')
                  r7 = nil
                end
                if r7
                  r0 = r7
                else
                  if input.index('double', index) == index
                    r8 = (SyntaxNode).new(input, index...(index + 6))
                    @index += 6
                  else
                    terminal_parse_failure('double')
                    r8 = nil
                  end
                  if r8
                    r0 = r8
                  else
                    if input.index('boolean', index) == index
                      r9 = (SyntaxNode).new(input, index...(index + 7))
                      @index += 7
                    else
                      terminal_parse_failure('boolean')
                      r9 = nil
                    end
                    if r9
                      r0 = r9
                    else
                      self.index = i0
                      r0 = nil
                    end
                  end
                end
              end
            end
          end
        end
      end
    end

    node_cache[:PrimitiveType][start_index] = r0

    return r0
  end

  module Dimensions0
  end

  def _nt_Dimensions
    start_index = index
    if node_cache[:Dimensions].has_key?(index)
      cached = node_cache[:Dimensions][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      i1, s1 = index, []
      if input.index('[', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('[')
        r2 = nil
      end
      s1 << r2
      if r2
        s3, i3 = [], index
        loop do
          if input.index(Regexp.new('[0-9]'), index) == index
            r4 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            r4 = nil
          end
          if r4
            s3 << r4
          else
            break
          end
        end
        r3 = SyntaxNode.new(input, i3...index, s3)
        s1 << r3
        if r3
          if input.index(']', index) == index
            r5 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            terminal_parse_failure(']')
            r5 = nil
          end
          s1 << r5
        end
      end
      if s1.last
        r1 = (SyntaxNode).new(input, i1...index, s1)
        r1.extend(Dimensions0)
      else
        self.index = i1
        r1 = nil
      end
      if r1
        s0 << r1
      else
        break
      end
    end
    if s0.empty?
      self.index = i0
      r0 = nil
    else
      r0 = SyntaxNode.new(input, i0...index, s0)
    end

    node_cache[:Dimensions][start_index] = r0

    return r0
  end

  module ClassType0
    def name
      elements[1]
    end
  end

  module ClassType1
    def name
      elements[0]
    end

  end

  def _nt_ClassType
    start_index = index
    if node_cache[:ClassType].has_key?(index)
      cached = node_cache[:ClassType][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_name
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        if input.index('::', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 2))
          @index += 2
        else
          terminal_parse_failure('::')
          r4 = nil
        end
        s3 << r4
        if r4
          r5 = _nt_name
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(ClassType0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(ClassType1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:ClassType][start_index] = r0

    return r0
  end

  module Constant0
    def StringConstant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant1
    def FloatConstant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant2
    def LongConstant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant3
    def IntegerConstant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant4
    def NullConstant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant5
    def BooleanConstant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant6
    def InfinityConstant
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant7
    def Vector
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  module Constant8
    def Matrix
      elements[0]
    end

    def Spacing
      elements[1]
    end
  end

  def _nt_Constant
    start_index = index
    if node_cache[:Constant].has_key?(index)
      cached = node_cache[:Constant][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    r2 = _nt_StringConstant
    s1 << r2
    if r2
      r3 = _nt_Spacing
      s1 << r3
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(Constant0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      i4, s4 = index, []
      r5 = _nt_FloatConstant
      s4 << r5
      if r5
        r6 = _nt_Spacing
        s4 << r6
      end
      if s4.last
        r4 = (SyntaxNode).new(input, i4...index, s4)
        r4.extend(Constant1)
      else
        self.index = i4
        r4 = nil
      end
      if r4
        r0 = r4
      else
        i7, s7 = index, []
        r8 = _nt_LongConstant
        s7 << r8
        if r8
          r9 = _nt_Spacing
          s7 << r9
        end
        if s7.last
          r7 = (SyntaxNode).new(input, i7...index, s7)
          r7.extend(Constant2)
        else
          self.index = i7
          r7 = nil
        end
        if r7
          r0 = r7
        else
          i10, s10 = index, []
          r11 = _nt_IntegerConstant
          s10 << r11
          if r11
            r12 = _nt_Spacing
            s10 << r12
          end
          if s10.last
            r10 = (SyntaxNode).new(input, i10...index, s10)
            r10.extend(Constant3)
          else
            self.index = i10
            r10 = nil
          end
          if r10
            r0 = r10
          else
            i13, s13 = index, []
            r14 = _nt_NullConstant
            s13 << r14
            if r14
              r15 = _nt_Spacing
              s13 << r15
            end
            if s13.last
              r13 = (SyntaxNode).new(input, i13...index, s13)
              r13.extend(Constant4)
            else
              self.index = i13
              r13 = nil
            end
            if r13
              r0 = r13
            else
              i16, s16 = index, []
              r17 = _nt_BooleanConstant
              s16 << r17
              if r17
                r18 = _nt_Spacing
                s16 << r18
              end
              if s16.last
                r16 = (SyntaxNode).new(input, i16...index, s16)
                r16.extend(Constant5)
              else
                self.index = i16
                r16 = nil
              end
              if r16
                r0 = r16
              else
                i19, s19 = index, []
                r20 = _nt_InfinityConstant
                s19 << r20
                if r20
                  r21 = _nt_Spacing
                  s19 << r21
                end
                if s19.last
                  r19 = (SyntaxNode).new(input, i19...index, s19)
                  r19.extend(Constant6)
                else
                  self.index = i19
                  r19 = nil
                end
                if r19
                  r0 = r19
                else
                  i22, s22 = index, []
                  r23 = _nt_Vector
                  s22 << r23
                  if r23
                    r24 = _nt_Spacing
                    s22 << r24
                  end
                  if s22.last
                    r22 = (SyntaxNode).new(input, i22...index, s22)
                    r22.extend(Constant7)
                  else
                    self.index = i22
                    r22 = nil
                  end
                  if r22
                    r0 = r22
                  else
                    i25, s25 = index, []
                    r26 = _nt_Matrix
                    s25 << r26
                    if r26
                      r27 = _nt_Spacing
                      s25 << r27
                    end
                    if s25.last
                      r25 = (SyntaxNode).new(input, i25...index, s25)
                      r25.extend(Constant8)
                    else
                      self.index = i25
                      r25 = nil
                    end
                    if r25
                      r0 = r25
                    else
                      self.index = i0
                      r0 = nil
                    end
                  end
                end
              end
            end
          end
        end
      end
    end

    node_cache[:Constant][start_index] = r0

    return r0
  end

  module ConstantList0
    def Constant
      elements[1]
    end
  end

  module ConstantList1
    def Constant
      elements[0]
    end

  end

  def _nt_ConstantList
    start_index = index
    if node_cache[:ConstantList].has_key?(index)
      cached = node_cache[:ConstantList][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_Constant
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        if input.index(',', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure(',')
          r4 = nil
        end
        s3 << r4
        if r4
          r5 = _nt_Constant
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(ConstantList0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(ConstantList1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:ConstantList][start_index] = r0

    return r0
  end

  def _nt_FloatConstant
    start_index = index
    if node_cache[:FloatConstant].has_key?(index)
      cached = node_cache[:FloatConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_FloatingPointString

    node_cache[:FloatConstant][start_index] = r0

    return r0
  end

  module FloatList0
    def FloatConstant
      elements[1]
    end
  end

  module FloatList1
    def FloatConstant
      elements[0]
    end

  end

  def _nt_FloatList
    start_index = index
    if node_cache[:FloatList].has_key?(index)
      cached = node_cache[:FloatList][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_FloatConstant
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        if input.index(',', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure(',')
          r4 = nil
        end
        s3 << r4
        if r4
          r5 = _nt_FloatConstant
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(FloatList0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(FloatList1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:FloatList][start_index] = r0

    return r0
  end

  module FloatingPointString0
  end

  module FloatingPointString1
  end

  module FloatingPointString2
    def Exponent
      elements[1]
    end

  end

  module FloatingPointString3
    def FloatTypeSuffix
      elements[2]
    end
  end

  def _nt_FloatingPointString
    start_index = index
    if node_cache[:FloatingPointString].has_key?(index)
      cached = node_cache[:FloatingPointString][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    i1, s1 = index, []
    s2, i2 = [], index
    loop do
      r3 = _nt_Digit
      if r3
        s2 << r3
      else
        break
      end
    end
    if s2.empty?
      self.index = i2
      r2 = nil
    else
      r2 = SyntaxNode.new(input, i2...index, s2)
    end
    s1 << r2
    if r2
      if input.index('.', index) == index
        r4 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('.')
        r4 = nil
      end
      s1 << r4
      if r4
        s5, i5 = [], index
        loop do
          r6 = _nt_Digit
          if r6
            s5 << r6
          else
            break
          end
        end
        if s5.empty?
          self.index = i5
          r5 = nil
        else
          r5 = SyntaxNode.new(input, i5...index, s5)
        end
        s1 << r5
        if r5
          r8 = _nt_Exponent
          if r8
            r7 = r8
          else
            r7 = SyntaxNode.new(input, index...index)
          end
          s1 << r7
          if r7
            r10 = _nt_FloatTypeSuffix
            if r10
              r9 = r10
            else
              r9 = SyntaxNode.new(input, index...index)
            end
            s1 << r9
          end
        end
      end
    end
    if s1.last
      r1 = (SyntaxNode).new(input, i1...index, s1)
      r1.extend(FloatingPointString0)
    else
      self.index = i1
      r1 = nil
    end
    if r1
      r0 = r1
    else
      i11, s11 = index, []
      if input.index('.', index) == index
        r12 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure('.')
        r12 = nil
      end
      s11 << r12
      if r12
        s13, i13 = [], index
        loop do
          r14 = _nt_Digit
          if r14
            s13 << r14
          else
            break
          end
        end
        if s13.empty?
          self.index = i13
          r13 = nil
        else
          r13 = SyntaxNode.new(input, i13...index, s13)
        end
        s11 << r13
        if r13
          r16 = _nt_Exponent
          if r16
            r15 = r16
          else
            r15 = SyntaxNode.new(input, index...index)
          end
          s11 << r15
          if r15
            r18 = _nt_FloatTypeSuffix
            if r18
              r17 = r18
            else
              r17 = SyntaxNode.new(input, index...index)
            end
            s11 << r17
          end
        end
      end
      if s11.last
        r11 = (SyntaxNode).new(input, i11...index, s11)
        r11.extend(FloatingPointString1)
      else
        self.index = i11
        r11 = nil
      end
      if r11
        r0 = r11
      else
        i19, s19 = index, []
        s20, i20 = [], index
        loop do
          r21 = _nt_Digit
          if r21
            s20 << r21
          else
            break
          end
        end
        if s20.empty?
          self.index = i20
          r20 = nil
        else
          r20 = SyntaxNode.new(input, i20...index, s20)
        end
        s19 << r20
        if r20
          r22 = _nt_Exponent
          s19 << r22
          if r22
            r24 = _nt_FloatTypeSuffix
            if r24
              r23 = r24
            else
              r23 = SyntaxNode.new(input, index...index)
            end
            s19 << r23
          end
        end
        if s19.last
          r19 = (SyntaxNode).new(input, i19...index, s19)
          r19.extend(FloatingPointString2)
        else
          self.index = i19
          r19 = nil
        end
        if r19
          r0 = r19
        else
          i25, s25 = index, []
          s26, i26 = [], index
          loop do
            r27 = _nt_Digit
            if r27
              s26 << r27
            else
              break
            end
          end
          if s26.empty?
            self.index = i26
            r26 = nil
          else
            r26 = SyntaxNode.new(input, i26...index, s26)
          end
          s25 << r26
          if r26
            r29 = _nt_Exponent
            if r29
              r28 = r29
            else
              r28 = SyntaxNode.new(input, index...index)
            end
            s25 << r28
            if r28
              r30 = _nt_FloatTypeSuffix
              s25 << r30
            end
          end
          if s25.last
            r25 = (SyntaxNode).new(input, i25...index, s25)
            r25.extend(FloatingPointString3)
          else
            self.index = i25
            r25 = nil
          end
          if r25
            r0 = r25
          else
            self.index = i0
            r0 = nil
          end
        end
      end
    end

    node_cache[:FloatingPointString][start_index] = r0

    return r0
  end

  module Exponent0
  end

  def _nt_Exponent
    start_index = index
    if node_cache[:Exponent].has_key?(index)
      cached = node_cache[:Exponent][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index(Regexp.new('[eE]'), index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r1 = nil
    end
    s0 << r1
    if r1
      if input.index(Regexp.new('[+\\-]'), index) == index
        r3 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        r3 = nil
      end
      if r3
        r2 = r3
      else
        r2 = SyntaxNode.new(input, index...index)
      end
      s0 << r2
      if r2
        s4, i4 = [], index
        loop do
          r5 = _nt_Digit
          if r5
            s4 << r5
          else
            break
          end
        end
        if s4.empty?
          self.index = i4
          r4 = nil
        else
          r4 = SyntaxNode.new(input, i4...index, s4)
        end
        s0 << r4
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Exponent0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:Exponent][start_index] = r0

    return r0
  end

  def _nt_FloatTypeSuffix
    start_index = index
    if node_cache[:FloatTypeSuffix].has_key?(index)
      cached = node_cache[:FloatTypeSuffix][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[fFdD]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:FloatTypeSuffix][start_index] = r0

    return r0
  end

  def _nt_IntegerConstant
    start_index = index
    if node_cache[:IntegerConstant].has_key?(index)
      cached = node_cache[:IntegerConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_HexConstant
    if r1
      r0 = r1
    else
      r2 = _nt_DecimalConstant
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:IntegerConstant][start_index] = r0

    return r0
  end

  module LongConstant0
    def DecimalConstant
      elements[0]
    end

    def LongTypeSuffix
      elements[1]
    end
  end

  def _nt_LongConstant
    start_index = index
    if node_cache[:LongConstant].has_key?(index)
      cached = node_cache[:LongConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_DecimalConstant
    s0 << r1
    if r1
      r2 = _nt_LongTypeSuffix
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(LongConstant0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:LongConstant][start_index] = r0

    return r0
  end

  module IntegerList0
    def IntegerConstant
      elements[1]
    end
  end

  module IntegerList1
    def IntegerConstant
      elements[0]
    end

  end

  def _nt_IntegerList
    start_index = index
    if node_cache[:IntegerList].has_key?(index)
      cached = node_cache[:IntegerList][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_IntegerConstant
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        if input.index(',', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure(',')
          r4 = nil
        end
        s3 << r4
        if r4
          r5 = _nt_IntegerConstant
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(IntegerList0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(IntegerList1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:IntegerList][start_index] = r0

    return r0
  end

  module HexConstant0
    def HexNumeral
      elements[0]
    end

  end

  def _nt_HexConstant
    start_index = index
    if node_cache[:HexConstant].has_key?(index)
      cached = node_cache[:HexConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_HexNumeral
    s0 << r1
    if r1
      r3 = _nt_HexTypeSuffix
      if r3
        r2 = r3
      else
        r2 = SyntaxNode.new(input, index...index)
      end
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(HexConstant0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:HexConstant][start_index] = r0

    return r0
  end

  module DecimalConstant0
    def DecimalNumeral
      elements[1]
    end
  end

  def _nt_DecimalConstant
    start_index = index
    if node_cache[:DecimalConstant].has_key?(index)
      cached = node_cache[:DecimalConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('-', index) == index
      r2 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('-')
      r2 = nil
    end
    if r2
      r1 = r2
    else
      r1 = SyntaxNode.new(input, index...index)
    end
    s0 << r1
    if r1
      r3 = _nt_DecimalNumeral
      s0 << r3
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(DecimalConstant0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:DecimalConstant][start_index] = r0

    return r0
  end

  module DecimalNumeral0
    def NonZeroDigit
      elements[0]
    end

  end

  def _nt_DecimalNumeral
    start_index = index
    if node_cache[:DecimalNumeral].has_key?(index)
      cached = node_cache[:DecimalNumeral][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('0', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('0')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      i2, s2 = index, []
      r3 = _nt_NonZeroDigit
      s2 << r3
      if r3
        s4, i4 = [], index
        loop do
          r5 = _nt_Digit
          if r5
            s4 << r5
          else
            break
          end
        end
        r4 = SyntaxNode.new(input, i4...index, s4)
        s2 << r4
      end
      if s2.last
        r2 = (SyntaxNode).new(input, i2...index, s2)
        r2.extend(DecimalNumeral0)
      else
        self.index = i2
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:DecimalNumeral][start_index] = r0

    return r0
  end

  def _nt_NonZeroDigit
    start_index = index
    if node_cache[:NonZeroDigit].has_key?(index)
      cached = node_cache[:NonZeroDigit][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[1-9]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:NonZeroDigit][start_index] = r0

    return r0
  end

  def _nt_Digit
    start_index = index
    if node_cache[:Digit].has_key?(index)
      cached = node_cache[:Digit][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[0-9]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:Digit][start_index] = r0

    return r0
  end

  module HexNumeral0
  end

  def _nt_HexNumeral
    start_index = index
    if node_cache[:HexNumeral].has_key?(index)
      cached = node_cache[:HexNumeral][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('0', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('0')
      r1 = nil
    end
    s0 << r1
    if r1
      if input.index(Regexp.new('[xX]'), index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        r2 = nil
      end
      s0 << r2
      if r2
        s3, i3 = [], index
        loop do
          r4 = _nt_HexDigit
          if r4
            s3 << r4
          else
            break
          end
        end
        if s3.empty?
          self.index = i3
          r3 = nil
        else
          r3 = SyntaxNode.new(input, i3...index, s3)
        end
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(HexNumeral0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:HexNumeral][start_index] = r0

    return r0
  end

  def _nt_HexDigit
    start_index = index
    if node_cache[:HexDigit].has_key?(index)
      cached = node_cache[:HexDigit][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[0-9a-fA-F]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:HexDigit][start_index] = r0

    return r0
  end

  def _nt_LongTypeSuffix
    start_index = index
    if node_cache[:LongTypeSuffix].has_key?(index)
      cached = node_cache[:LongTypeSuffix][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[lL]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:LongTypeSuffix][start_index] = r0

    return r0
  end

  def _nt_HexTypeSuffix
    start_index = index
    if node_cache[:HexTypeSuffix].has_key?(index)
      cached = node_cache[:HexTypeSuffix][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index(Regexp.new('[UI]'), index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r0 = nil
    end

    node_cache[:HexTypeSuffix][start_index] = r0

    return r0
  end

  def _nt_StringConstant
    start_index = index
    if node_cache[:StringConstant].has_key?(index)
      cached = node_cache[:StringConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    r0 = _nt_CharacterSequence

    node_cache[:StringConstant][start_index] = r0

    return r0
  end

  module CharacterSequence0
  end

  module CharacterSequence1
  end

  def _nt_CharacterSequence
    start_index = index
    if node_cache[:CharacterSequence].has_key?(index)
      cached = node_cache[:CharacterSequence][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index(Regexp.new('["]'), index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      r1 = nil
    end
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3 = index
        r4 = _nt_EscapeSequence
        if r4
          r3 = r4
        else
          i5, s5 = index, []
          i6 = index
          if input.index(Regexp.new('["\\\\]'), index) == index
            r7 = (SyntaxNode).new(input, index...(index + 1))
            @index += 1
          else
            r7 = nil
          end
          if r7
            r6 = nil
          else
            self.index = i6
            r6 = SyntaxNode.new(input, index...index)
          end
          s5 << r6
          if r6
            if index < input_length
              r8 = (SyntaxNode).new(input, index...(index + 1))
              @index += 1
            else
              terminal_parse_failure("any character")
              r8 = nil
            end
            s5 << r8
          end
          if s5.last
            r5 = (SyntaxNode).new(input, i5...index, s5)
            r5.extend(CharacterSequence0)
          else
            self.index = i5
            r5 = nil
          end
          if r5
            r3 = r5
          else
            self.index = i3
            r3 = nil
          end
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
      if r2
        if input.index(Regexp.new('["]'), index) == index
          r9 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          r9 = nil
        end
        s0 << r9
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(CharacterSequence1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:CharacterSequence][start_index] = r0

    return r0
  end

  module EscapeSequence0
  end

  def _nt_EscapeSequence
    start_index = index
    if node_cache[:EscapeSequence].has_key?(index)
      cached = node_cache[:EscapeSequence][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('\\', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('\\')
      r1 = nil
    end
    s0 << r1
    if r1
      if input.index(Regexp.new('[btnfr"\'\\\\]'), index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        r2 = nil
      end
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(EscapeSequence0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:EscapeSequence][start_index] = r0

    return r0
  end

  def _nt_BooleanConstant
    start_index = index
    if node_cache[:BooleanConstant].has_key?(index)
      cached = node_cache[:BooleanConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('true', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 4))
      @index += 4
    else
      terminal_parse_failure('true')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('false', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 5))
        @index += 5
      else
        terminal_parse_failure('false')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:BooleanConstant][start_index] = r0

    return r0
  end

  def _nt_InfinityConstant
    start_index = index
    if node_cache[:InfinityConstant].has_key?(index)
      cached = node_cache[:InfinityConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index('infinity', index) == index
      r0 = (SyntaxNode).new(input, index...(index + 8))
      @index += 8
    else
      terminal_parse_failure('infinity')
      r0 = nil
    end

    node_cache[:InfinityConstant][start_index] = r0

    return r0
  end

  def _nt_NullConstant
    start_index = index
    if node_cache[:NullConstant].has_key?(index)
      cached = node_cache[:NullConstant][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('null', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 4))
      @index += 4
    else
      terminal_parse_failure('null')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('nil', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 3))
        @index += 3
      else
        terminal_parse_failure('nil')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:NullConstant][start_index] = r0

    return r0
  end

  def _nt_Vector
    start_index = index
    if node_cache[:Vector].has_key?(index)
      cached = node_cache[:Vector][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_IntVector
    if r1
      r0 = r1
    else
      r2 = _nt_FloatVector
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Vector][start_index] = r0

    return r0
  end

  module IntVector0
    def IntegerList
      elements[1]
    end

  end

  def _nt_IntVector
    start_index = index
    if node_cache[:IntVector].has_key?(index)
      cached = node_cache[:IntVector][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('{', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('{')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_IntegerList
      s0 << r2
      if r2
        if input.index('}', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('}')
          r3 = nil
        end
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(IntVector0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:IntVector][start_index] = r0

    return r0
  end

  module FloatVector0
    def FloatList
      elements[1]
    end

  end

  def _nt_FloatVector
    start_index = index
    if node_cache[:FloatVector].has_key?(index)
      cached = node_cache[:FloatVector][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('{', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('{')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_FloatList
      s0 << r2
      if r2
        if input.index('}', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('}')
          r3 = nil
        end
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(FloatVector0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:FloatVector][start_index] = r0

    return r0
  end

  def _nt_Matrix
    start_index = index
    if node_cache[:Matrix].has_key?(index)
      cached = node_cache[:Matrix][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_IntMatrix
    if r1
      r0 = r1
    else
      r2 = _nt_FloatMatrix
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:Matrix][start_index] = r0

    return r0
  end

  module IntMatrix0
    def IntMatrixEntries
      elements[1]
    end

  end

  def _nt_IntMatrix
    start_index = index
    if node_cache[:IntMatrix].has_key?(index)
      cached = node_cache[:IntMatrix][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('{', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('{')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_IntMatrixEntries
      s0 << r2
      if r2
        if input.index('}', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('}')
          r3 = nil
        end
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(IntMatrix0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:IntMatrix][start_index] = r0

    return r0
  end

  module FloatMatrix0
    def FloatMatrixEntries
      elements[1]
    end

  end

  def _nt_FloatMatrix
    start_index = index
    if node_cache[:FloatMatrix].has_key?(index)
      cached = node_cache[:FloatMatrix][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('{', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('{')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_FloatMatrixEntries
      s0 << r2
      if r2
        if input.index('}', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure('}')
          r3 = nil
        end
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(FloatMatrix0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:FloatMatrix][start_index] = r0

    return r0
  end

  module IntMatrixEntries0
    def IntVector
      elements[1]
    end
  end

  module IntMatrixEntries1
    def IntVector
      elements[0]
    end

  end

  def _nt_IntMatrixEntries
    start_index = index
    if node_cache[:IntMatrixEntries].has_key?(index)
      cached = node_cache[:IntMatrixEntries][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_IntVector
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        if input.index(',', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure(',')
          r4 = nil
        end
        s3 << r4
        if r4
          r5 = _nt_IntVector
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(IntMatrixEntries0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(IntMatrixEntries1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:IntMatrixEntries][start_index] = r0

    return r0
  end

  module FloatMatrixEntries0
    def FloatVector
      elements[1]
    end
  end

  module FloatMatrixEntries1
    def FloatVector
      elements[0]
    end

  end

  def _nt_FloatMatrixEntries
    start_index = index
    if node_cache[:FloatMatrixEntries].has_key?(index)
      cached = node_cache[:FloatMatrixEntries][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_FloatVector
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        if input.index(',', index) == index
          r4 = (SyntaxNode).new(input, index...(index + 1))
          @index += 1
        else
          terminal_parse_failure(',')
          r4 = nil
        end
        s3 << r4
        if r4
          r5 = _nt_FloatVector
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(FloatMatrixEntries0)
        else
          self.index = i3
          r3 = nil
        end
        if r3
          s2 << r3
        else
          break
        end
      end
      r2 = SyntaxNode.new(input, i2...index, s2)
      s0 << r2
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(FloatMatrixEntries1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:FloatMatrixEntries][start_index] = r0

    return r0
  end

end

class OverlogParser < Treetop::Runtime::CompiledParser
  include Overlog
end

