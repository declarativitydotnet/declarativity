#!/usr/bin/env ruby
nruns = 4
#sizes = [10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40]
sizes = [10,12,14,16,18]
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
