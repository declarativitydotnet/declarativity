
LPATH = "~/code/path-labeling"
CXT = "./code"


fp = File.open("scripts/mods.cfg", "r")
while (str = fp.gets)
  file, mod = str.split(/\s+/)
  dn = File.dirname(file)
  exe = "ruby -I . -I #{CXT} -I #{dn} -I #{LPATH} #{LPATH}/bin/label -r #{file} -i #{mod} -P"
  #puts "EXE #{exe}"
  #res = `#{exe}`
  IO::popen(exe) do |io|
    while (ln = io.gets)
      next if ln =~ /^Warning/
      puts "#{file}::#{mod}\t#{ln}"
    end
  end
end
fp.close
