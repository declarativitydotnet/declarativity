require 'rubygems' 
spec = Gem::Specification.new do |s| 
  s.name = "Lincoln" 
  s.version = "0.0.1" 
  s.author = "Tyson Condie and Joe Hellerstein" 
  s.email = "lincoln_dev@db.cs.berkeley.edu" 
  s.homepage = "http://www.declarativity.net" 
  s.platform = Gem::Platform::RUBY 
  s.summary = "Ruby implementation of a runtime for Lincoln, a declarative language for parallel and distributed computing." 
  s.files = [] 
  s.require_path = "." 
  s.autorequire = "Lincoln" 
  s.extensions = [] 
end 
if $0 == __FILE__ 
  Gem::manage_gems 
  Gem::Builder.new(spec).build 
end 
