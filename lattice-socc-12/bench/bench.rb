#!/usr/bin/env ruby
require "rubygems"
require "benchmark"
require "bud"

class AllPathsL
  include Bud

  state do
    lhset :link
    lhset :path
  end

  bloom do
    path <= link
    path <= path.eqjoin(link, 1, 0) do |p,l|
      [p[0], l[1], p[2] + l[2]]
    end
  end
end

class AllPathsB
  include Bud

  state do
    table :link, [:from, :to, :cost]
    table :path, [:from, :to, :cost]
  end

  bloom do
    path <= link
    path <= (path * link).pairs(:to => :from) do |p,l|
      [p.from, l.to, p.cost + l.cost]
    end
  end
end

def log_base_2(n)
  Math.log(n) / Math.log(2)
end

def gen_link_data(num_nodes)
  # Note that we represent node IDs using symbols. This actually makes a huge
  # performance difference because Ruby Strings are mutable, whereas Symbols are
  # not. Hence copying strings is much more expensive, as is comparing them
  # (comparing symbols can just look at the addresses, not the symbol
  # content). This change improves the performance of both Bloom and the
  # lattice-based approach, but it helps a LOT more for lattices.
  nodes = 1.upto(num_nodes).map {|i| "n#{i}".to_sym}
  links = []
  layers = log_base_2(num_nodes).to_i
  nodes.each_with_index do |n,i|
    layers.times do |j|
      dest = (2 ** j) + i
      next if dest >= nodes.length
      links << [n, nodes[dest], 1]
    end
  end
  links
end

def lattice_bench(data, use_naive=false)
  l = AllPathsL.new(:disable_lattice_semi_naive => use_naive)
  l.link <+ [data]
  t = Benchmark.realtime do
    l.tick
  end
  npaths = l.path.current_value.reveal.length
  l.stop
  [t, npaths]
end

def bloom_bench(data)
  b = AllPathsB.new
  b.link <+ data
  t = Benchmark.realtime do
    b.tick
  end
  npaths = b.path.to_a.length
  b.stop
  [t, npaths]
end

def report_mem
  `ps -o rss= -p #{Process.pid}`.to_i
end

def bench(size, run_idx, variant)
  # Don't try to use naive evaluation for large graphs
  return if (variant == "naive-lat" && size > 64)

  data = gen_link_data(size)
  puts "****** #{Time.now} ******"
  puts "Run #: #{run_idx}, size = #{size}, # links = #{data.length}, variant = #{variant}"

  case variant
  when "bloom"
    elapsed_t, npaths = bloom_bench(data)
  when "seminaive-lat"
    elapsed_t, npaths = lattice_bench(data)
  when "naive-lat"
    elapsed_t, npaths = lattice_bench(data, true)
  else
    raise "Unrecognized variant: #{variant}"
  end

  puts "Elapsed time: #{elapsed_t}, # of paths: #{npaths}"
  $stderr.printf("%d %d %d %.6f\n",
                 size, data.length, npaths, elapsed_t)
end

raise ArgumentError, "Usage: bench.rb graph_size run_idx variant" unless ARGV.length == 3
size, run_idx, variant = ARGV
bench(size.to_i, run_idx.to_i, variant)
