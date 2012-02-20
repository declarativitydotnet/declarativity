#!/usr/bin/env ruby
nruns = 4
#sizes = [10, 12, 14, 16, 18, 20, 22]
sizes = [10]
data_file = "perf.data"
log_file = "exp_log"

`rm -f #{log_file}`
`echo "#NPaths Bloom LatticeSN" | cat > #{data_file}`

sizes.each do |s|
  `ruby bench.rb #{s} #{nruns} >> #{log_file} 2>>#{data_file}`
end
