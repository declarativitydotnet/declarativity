# pingpong demo
# To run:
#  fire up a ponger with 'ruby ponger.rb 12346 127.0.0.1 12345'
#  fire up a pinger with 'ruby pinger.rb 12345 2 127.0.0.1 12346'
#  you should see packets received on either side
require 'rubygems'
require 'bloom'

class Ponger < Bloom
  attr_reader :myloc
  attr_reader :otherloc

  def initialize(ip, port)
    super ip, port
    @otherip = ARGV[1]
    @otherport = ARGV[2]
    @myloc = "#{ip.to_s}:#{port.to_s}"
    @otherloc = "#{@otherip}:#{@otherport}"
  end

  def state
    channel :pingpongs, 0, ['otherloc', 'myloc', 'msg', 'wall', 'bloom']
  end

  def declaration
    strata[0] = rules {
      # whenever we get a ping, send a pong
      pingpongs <+ pingpongs.map {|p| [@otherloc, @myloc, 'pong!', Time.new.to_s, bloomtime]}      
    }
  end
end

program = Ponger.new('127.0.0.1', ARGV[0])
program.run
