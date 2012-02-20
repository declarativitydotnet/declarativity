#!/usr/bin/env ruby
require "rubygems"
require "benchmark"
require "bud"

class AllPathsL
  include Bud

  state do
    lset :link
    lset :path, :scratch => true
  end

  bloom do
    path <= link
    path <= path.product(link).pro do |p,l|
      [[p[0], l[1], p[2] + l[2]]] if p[1] == l[0]
    end
  end
end

class AllPathsB
  include Bud

  state do
    table :link, [:from, :to, :cost]
    scratch :path, [:from, :to, :cost]
  end

  bloom do
    path <= link
    path <= (path * link).pairs do |p,l|
      [p.from, l.to, p.cost + l.cost] if p.to == l.from
    end
  end
end

def log_base_2(n)
  Math.log(n) / Math.log(2)
end

def gen_link_data(num_nodes)
  nodes = 1.upto(num_nodes).map {|i| "n#{i}"}
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

def lattice_bench(data, nruns, use_naive=false)
  l = AllPathsL.new(:disable_lattice_semi_naive => use_naive)
  l.link <+ [data]
  total_time = 0.0
  npaths = nil
  nruns.times do |i|
    t = Benchmark.realtime do
      l.tick
    end
    total_time += t
    new_npaths = l.path.current_value.reveal.length
    puts "#{use_naive ? "Naive" : "SN"} lattice done #{i+1}/#{nruns}; #{t} seconds. # of paths: #{new_npaths} (RSS size: #{report_mem})"
    if npaths != nil && npaths != new_npaths
      raise "Non-deterministic results: old = #{npaths}, new = #{new_npaths}"
    end
    npaths = new_npaths
  end
  l.stop
  [total_time / nruns, npaths]
end

def bloom_bench(data, nruns)
  b = AllPathsB.new
  b.link <+ data
  total_time = 0.0
  npaths = nil
  nruns.times do |i|
    t = Benchmark.realtime do
      b.tick
    end
    total_time += t
    puts "Bloom done #{i+1}/#{nruns}; #{t} seconds. # of paths: #{b.path.to_a.length} (RSS size: #{report_mem})"
    if npaths != nil && npaths != b.path.to_a.length
      raise "Non-deterministic results: old = #{npaths}, new = #{b.path.to_a.length}"
    end
    npaths = b.path.to_a.length
  end
  
  b.stop
  [total_time / nruns, npaths]
end

def report_mem
  `ps -o rss= -p #{Process.pid}`.to_i
end

def bench(size, nruns)
  data = gen_link_data(size)
  puts "****** #{Time.now} ******"
  puts "Running bench for size = #{size}, # links = #{data.length}"
  puts "(avg links/node: #{data.length.to_f/size})"

  t1, npath1 = bloom_bench(data, nruns)
  GC.start
  t2, npath2 = lattice_bench(data, nruns)
  GC.start

  # Don't try to use naive evaluation for large graphs
  if size <= 18
    t3, npath3 = lattice_bench(data, nruns, true)
  end

  unless npath1 == npath2 && (npath1 == npath3 || npath3.nil?)
    raise "ERROR, path mismatch. #{npath1}, #{npath2}, #{npath3}"
  end

  puts "Results: avg bloom = #{t1}, avg sn lattice = #{t2}, avg naive lattice = #{t3}"
  $stderr.printf("%d %.6f %.6f", npath1, t1, t2)
  unless t3.nil?
    $stderr.printf(" %0.6f", t3)
  end
  $stderr.printf("\n")
end

raise ArgumentError, "Usage: bench.rb graph_size nruns" unless ARGV.length == 2
size, nruns = ARGV
bench(size.to_i, nruns.to_i)
