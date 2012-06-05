#!/usr/bin/env ruby
nruns = 5
#sizes = [32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112]
sizes = [32]
variants = ["bloom", "seminaive-lat"]#, "naive-lat"]
data_files = {}
variants.each {|v| data_files[v] = "#{v}.data"}
log_file = "exp_log"

`rm -f #{log_file}`
data_files.each_pair do |k, fname|
  `echo "#Variant: #{k}\n#NNodes NLinks NPaths Runtime" | cat > #{fname}`
end

sizes.each do |s|
  puts "Running benchmark; size = #{s}"
  variants.each do |v|
    print "  #{v}"
    nruns.times do |t|
      `ruby bench.rb #{s} #{t} #{v} >> #{log_file} 2>>#{data_files[v]}`
      print "."
    end
    print "\n"
  end
end

module Enumerable
  def sum
    self.inject(0){|accum, i| accum + i }
  end

  def mean
    self.sum/self.length.to_f
  end

  def sample_variance
    m = self.mean
    sum = self.inject(0){|accum, i| accum +(i-m)**2 }
    sum/(self.length - 1).to_f
  end

  def standard_deviation
    return Math.sqrt(self.sample_variance)
  end
end

# Compute summary data for each run. It would be more convenient to compute this
# in the gnuplot script, but it seems hard to do a grouped aggregate in gnuplot.
data_files.each_pair do |v, fname|
  File.open("#{v}.summary", "w") do |n|
    n.puts "#Variant: #{v}"
    n.puts "#NNodes NLinks NPaths MeanRuntime StdDev StdError"
    groups = {}
    File.open(fname, "r").each_line do |l|
      next if l =~ /^#/
      fields = l.split(" ")
      graph_sizes = fields[0..-2].map {|x| x.to_i}
      runtime = fields.last.to_f
      groups[graph_sizes] ||= []
      groups[graph_sizes] << runtime
    end
    groups.keys.sort.each do |k|
      time_ary = groups[k]
      std_dev = time_ary.standard_deviation
      std_error = std_dev / Math.sqrt(time_ary.length.to_f)
      n.printf("%d %d %d %0.6f %0.6f %0.6f\n",
               k[0], k[1], k[2], time_ary.mean, std_dev, std_error)
    end
  end
end
