#!/usr/bin/env ruby
nruns = 5
sizes = [20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120]
variants = ["bloom", "seminaive-lat", "naive-lat"]
data_files = {}
variants.each {|v| data_files[v] = "#{v}.data"}
log_file = "exp_log"

`rm -f #{log_file}`
data_files.each {|v,f| `echo "#Variant: #{v}\n#NNodes NLinks NPaths MeanRuntime Stddev StdError" | cat > #{f}`}

sizes.each do |s|
  puts "Running benchmark; size = #{s}..."
  variants.each do |v|
    `ruby bench.rb #{s} #{nruns} #{v} >> #{log_file} 2>>#{data_files[v]}`
  end
end
