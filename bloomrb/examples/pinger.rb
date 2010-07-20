# pingpong demo #1
# This demo uses separate scripts for pinger and ponger.
# To run:
#  fire up a ponger with 'ruby ponger.rb 12346 127.0.0.1 12345'
#  fire up a pinger with 'ruby pinger.rb 12345 2 127.0.0.1 12346'
#  you should see packets received on either side
require 'rubygems'
require 'bloom'

class Pinger < Bloom
  attr_reader :myloc
  attr_reader :otherloc

  def initialize(ip, port)
    super ip, port
    @otherip = ARGV[2]
    @otherport = ARGV[3]
    @myloc = "#{ip.to_s}:#{port.to_s}"
    @otherloc = "#{@otherip}:#{@otherport}"
  end

  def state
    channel :pingpongs, 0, ['otherloc', 'myloc', 'msg', 'wall', 'bloom']
    periodic :timer, ARGV[1]
  end

  def declaration
    strata[0] = rules {
      # whenever we get a timer, send out a tuple
      pingpongs <+ timer.map {|t| [@otherloc, @myloc, 'ping!', t.time, bloomtime]}      
    }
  end
end

program = Pinger.new('127.0.0.1', ARGV[0])
program.run
