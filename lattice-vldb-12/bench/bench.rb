require "rubygems"
require "benchmark"
require "bud"

class AllPathsL
  include Bud

  state do
    lset :link
    lset :path, :scratch => false
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
    table :path, [:from, :to, :cost]
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
  nruns.times do
    t = Benchmark.realtime do
      l.tick
    end
    puts "Lattice done; #{t} seconds. # of paths: #{l.path.current_value.reveal.length}"
    report_mem
  end
  l.stop
end

def bloom_bench(data, nruns)
  b = AllPathsB.new
  b.link <+ data
  nruns.times do
    t = Benchmark.realtime do
      b.tick
    end
    puts "Bloom done; #{t} seconds. # of paths: #{b.path.to_a.length}"
    report_mem
  end
  b.stop
end

def report_mem
  rss_size = `ps -o rss= -p #{Process.pid}`.to_i
  puts "Process RSS (kB): #{rss_size}"
end

def bench(size, nruns)
  data = gen_link_data(size)
  puts "Running bench for size = #{size}, # links = #{data.length}"
  puts "(avg links/node: #{data.length.to_f/size})"

  bloom_bench(data, nruns)
  GC.start
  lattice_bench(data, nruns)
end

bench(20, 4)
