#!/usr/bin/env ruby
nruns = 4
sizes = [10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40]
data_file = "perf.data"
log_file = "exp_log"

`rm -f #{log_file}`
`echo "#NNodes NLinks NPaths Bloom LatticeSemi LatticeNaive" | cat > #{data_file}`

sizes.each do |s|
  puts "Running benchmark; size = #{s}..."
  `ruby bench.rb #{s} #{nruns} >> #{log_file} 2>>#{data_file}`
end
