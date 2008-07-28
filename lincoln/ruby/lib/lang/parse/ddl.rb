module Ddl
  include Treetop::Runtime

  def root
    @root || :program
  end

  module Program0
    def s
      elements[0]
    end

    def statement
      elements[1]
    end
  end

  def _nt_program
    start_index = index
    if node_cache[:program].has_key?(index)
      cached = node_cache[:program][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      i1, s1 = index, []
      r2 = _nt_s
      s1 << r2
      if r2
        r3 = _nt_statement
        s1 << r3
      end
      if s1.last
        r1 = (SyntaxNode).new(input, i1...index, s1)
        r1.extend(Program0)
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

    node_cache[:program][start_index] = r0

    return r0
  end

  module Statement0
    def s
      elements[1]
    end

    def tablename
      elements[2]
    end

    def opar
      elements[3]
    end

    def collist
      elements[4]
    end

    def cpar
      elements[5]
    end

    def acabo
      elements[7]
    end
  end

  def _nt_statement
    start_index = index
    if node_cache[:statement].has_key?(index)
      cached = node_cache[:statement][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    if input.index('table', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 5))
      @index += 5
    else
      terminal_parse_failure('table')
      r1 = nil
    end
    s0 << r1
    if r1
      r2 = _nt_s
      s0 << r2
      if r2
        r3 = _nt_tablename
        s0 << r3
        if r3
          r4 = _nt_opar
          s0 << r4
          if r4
            r5 = _nt_collist
            s0 << r5
            if r5
              r6 = _nt_cpar
              s0 << r6
              if r6
                r8 = _nt_keys
                if r8
                  r7 = r8
                else
                  r7 = SyntaxNode.new(input, index...index)
                end
                s0 << r7
                if r7
                  r9 = _nt_acabo
                  s0 << r9
                end
              end
            end
          end
        end
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Statement0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:statement][start_index] = r0

    return r0
  end

  module Collist0
    def comma
      elements[0]
    end

    def column
      elements[1]
    end
  end

  module Collist1
    def column
      elements[0]
    end

  end

  def _nt_collist
    start_index = index
    if node_cache[:collist].has_key?(index)
      cached = node_cache[:collist][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_column
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        r4 = _nt_comma
        s3 << r4
        if r4
          r5 = _nt_column
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(Collist0)
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
      r0.extend(Collist1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:collist][start_index] = r0

    return r0
  end

  module Column0
    def key_colname
      elements[0]
    end

    def s
      elements[1]
    end

    def dtype
      elements[2]
    end
  end

  def _nt_column
    start_index = index
    if node_cache[:column].has_key?(index)
      cached = node_cache[:column][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_key_colname
    s0 << r1
    if r1
      r2 = _nt_s
      s0 << r2
      if r2
        r3 = _nt_dtype
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Column0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:column][start_index] = r0

    return r0
  end

  module KeyColname0
    def key_modifier
      elements[0]
    end

    def colname
      elements[1]
    end
  end

  def _nt_key_colname
    start_index = index
    if node_cache[:key_colname].has_key?(index)
      cached = node_cache[:key_colname][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_colname
    if r1
      r0 = r1
    else
      i2, s2 = index, []
      r3 = _nt_key_modifier
      s2 << r3
      if r3
        r4 = _nt_colname
        s2 << r4
      end
      if s2.last
        r2 = (SyntaxNode).new(input, i2...index, s2)
        r2.extend(KeyColname0)
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

    node_cache[:key_colname][start_index] = r0

    return r0
  end

  def _nt_colname
    start_index = index
    if node_cache[:colname].has_key?(index)
      cached = node_cache[:colname][index]
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
    if s0.empty?
      self.index = i0
      r0 = nil
    else
      r0 = SyntaxNode.new(input, i0...index, s0)
    end

    node_cache[:colname][start_index] = r0

    return r0
  end

  def _nt_tablename
    start_index = index
    if node_cache[:tablename].has_key?(index)
      cached = node_cache[:tablename][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      if input.index(Regexp.new('[a-zA-Z0-9]'), index) == index
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

    node_cache[:tablename][start_index] = r0

    return r0
  end

  def _nt_dtype
    start_index = index
    if node_cache[:dtype].has_key?(index)
      cached = node_cache[:dtype][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    if input.index('String', index) == index
      r1 = (SyntaxNode).new(input, index...(index + 6))
      @index += 6
    else
      terminal_parse_failure('String')
      r1 = nil
    end
    if r1
      r0 = r1
    else
      if input.index('Integer', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 7))
        @index += 7
      else
        terminal_parse_failure('Integer')
        r2 = nil
      end
      if r2
        r0 = r2
      else
        if input.index('Float', index) == index
          r3 = (SyntaxNode).new(input, index...(index + 5))
          @index += 5
        else
          terminal_parse_failure('Float')
          r3 = nil
        end
        if r3
          r0 = r3
        else
          if input.index('Object', index) == index
            r4 = (SyntaxNode).new(input, index...(index + 6))
            @index += 6
          else
            terminal_parse_failure('Object')
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

    node_cache[:dtype][start_index] = r0

    return r0
  end

  module Keys0
    def s
      elements[0]
    end

    def opar
      elements[2]
    end

    def keylist
      elements[3]
    end

    def cpar
      elements[4]
    end
  end

  def _nt_keys
    start_index = index
    if node_cache[:keys].has_key?(index)
      cached = node_cache[:keys][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_s
    s0 << r1
    if r1
      if input.index('keys', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 4))
        @index += 4
      else
        terminal_parse_failure('keys')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_opar
        s0 << r3
        if r3
          r4 = _nt_keylist
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
      r0.extend(Keys0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:keys][start_index] = r0

    return r0
  end

  def _nt_keylist
    start_index = index
    if node_cache[:keylist].has_key?(index)
      cached = node_cache[:keylist][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0 = index
    r1 = _nt_numlist
    if r1
      r0 = r1
    else
      r2 = _nt_strlist
      if r2
        r0 = r2
      else
        self.index = i0
        r0 = nil
      end
    end

    node_cache[:keylist][start_index] = r0

    return r0
  end

  module Numlist0
    def comma
      elements[0]
    end

    def num
      elements[1]
    end
  end

  module Numlist1
    def num
      elements[0]
    end

  end

  def _nt_numlist
    start_index = index
    if node_cache[:numlist].has_key?(index)
      cached = node_cache[:numlist][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_num
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        r4 = _nt_comma
        s3 << r4
        if r4
          r5 = _nt_num
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(Numlist0)
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
      r0.extend(Numlist1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:numlist][start_index] = r0

    return r0
  end

  module Strlist0
    def comma
      elements[0]
    end

    def str
      elements[1]
    end
  end

  module Strlist1
    def str
      elements[0]
    end

  end

  def _nt_strlist
    start_index = index
    if node_cache[:strlist].has_key?(index)
      cached = node_cache[:strlist][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_str
    s0 << r1
    if r1
      s2, i2 = [], index
      loop do
        i3, s3 = index, []
        r4 = _nt_comma
        s3 << r4
        if r4
          r5 = _nt_str
          s3 << r5
        end
        if s3.last
          r3 = (SyntaxNode).new(input, i3...index, s3)
          r3.extend(Strlist0)
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
      r0.extend(Strlist1)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:strlist][start_index] = r0

    return r0
  end

  def _nt_num
    start_index = index
    if node_cache[:num].has_key?(index)
      cached = node_cache[:num][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      if input.index(Regexp.new('[0-9]'), index) == index
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

    node_cache[:num][start_index] = r0

    return r0
  end

  def _nt_str
    start_index = index
    if node_cache[:str].has_key?(index)
      cached = node_cache[:str][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      if input.index(Regexp.new('[a-zA-Z0-9]'), index) == index
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

    node_cache[:str][start_index] = r0

    return r0
  end

  module Comma0
    def s
      elements[0]
    end

    def s
      elements[2]
    end
  end

  def _nt_comma
    start_index = index
    if node_cache[:comma].has_key?(index)
      cached = node_cache[:comma][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_s
    s0 << r1
    if r1
      if input.index(',', index) == index
        r2 = (SyntaxNode).new(input, index...(index + 1))
        @index += 1
      else
        terminal_parse_failure(',')
        r2 = nil
      end
      s0 << r2
      if r2
        r3 = _nt_s
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

    node_cache[:comma][start_index] = r0

    return r0
  end

  module Acabo0
    def s
      elements[0]
    end

    def s
      elements[2]
    end
  end

  def _nt_acabo
    start_index = index
    if node_cache[:acabo].has_key?(index)
      cached = node_cache[:acabo][index]
      @index = cached.interval.end if cached
      return cached
    end

    i0, s0 = index, []
    r1 = _nt_s
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
        r3 = _nt_s
        s0 << r3
      end
    end
    if s0.last
      r0 = (SyntaxNode).new(input, i0...index, s0)
      r0.extend(Acabo0)
    else
      self.index = i0
      r0 = nil
    end

    node_cache[:acabo][start_index] = r0

    return r0
  end

  def _nt_key_modifier
    start_index = index
    if node_cache[:key_modifier].has_key?(index)
      cached = node_cache[:key_modifier][index]
      @index = cached.interval.end if cached
      return cached
    end

    if input.index('+', index) == index
      r0 = (SyntaxNode).new(input, index...(index + 1))
      @index += 1
    else
      terminal_parse_failure('+')
      r0 = nil
    end

    node_cache[:key_modifier][start_index] = r0

    return r0
  end

  module Opar0
    def s
      elements[0]
    end

    def s
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
    r1 = _nt_s
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
        r3 = _nt_s
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
    def s
      elements[0]
    end

    def s
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
    r1 = _nt_s
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
        r3 = _nt_s
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

  def _nt_s
    start_index = index
    if node_cache[:s].has_key?(index)
      cached = node_cache[:s][index]
      @index = cached.interval.end if cached
      return cached
    end

    s0, i0 = [], index
    loop do
      if input.index(Regexp.new('[ \\t\\n]'), index) == index
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

    node_cache[:s][start_index] = r0

    return r0
  end

end

class DdlParser < Treetop::Runtime::CompiledParser
  include Ddl
end

