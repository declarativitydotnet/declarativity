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
    path <= (path * link).pairs do |l,p|
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
  nruns.times do
    t = Benchmark.realtime do
      l.tick
    end
    total_time += t
    puts "Lattice done; #{t} seconds. # of paths: #{l.path.current_value.reveal.length}"
    report_mem
  end
  l.stop
  total_time / nruns
end

def bloom_bench(data, nruns)
  b = AllPathsB.new
  b.link <+ data
  total_time = 0.0
  nruns.times do
    t = Benchmark.realtime do
      b.tick
    end
    total_time += t
    puts "Bloom done; #{t} seconds. # of paths: #{b.path.to_a.length}"
    report_mem
  end
  b.stop
  total_time / nruns
end

def report_mem
  rss_size = `ps -o rss= -p #{Process.pid}`.to_i
  puts "Process RSS (kB): #{rss_size}"
end

def bench(size, nruns)
  data = gen_link_data(size)
  puts "****** #{Time.now} ******"
  puts "Running bench for size = #{size}, # links = #{data.length}"
  puts "(avg links/node: #{data.length.to_f/size})"

  t1 = bloom_bench(data, nruns)
  GC.start
  t2 = lattice_bench(data, nruns)

  puts "Results: avg bloom = #{t1}, avg lattice = #{t2}"
  $stderr.printf("%d %.6f %.6f\n", data.length, t1, t2)
end

raise ArgumentError, "Usage: bench.rb graph_size nruns" unless ARGV.length == 2
size, nruns = ARGV
bench(size.to_i, nruns.to_i)
