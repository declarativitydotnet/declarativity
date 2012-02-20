#!/usr/bin/env ruby
nruns = 4
sizes = [10, 12, 14, 16]
data_file = "perf.data"
log_file = "exp_log"

`rm -f #{log_file}`
`echo "#NPaths Bloom LatticeSN" | cat > #{data_file}`

sizes.each do |s|
  puts "Running benchmark; size = #{s}..."
  `ruby bench.rb #{s} #{nruns} >> #{log_file} 2>>#{data_file}`
end
