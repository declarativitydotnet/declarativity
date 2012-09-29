
LPATH = "~/code/path-labeling"
CXT = "./code"

flags = ARGV[0]
flags = "P" if flags.nil?

def table_row(cols)
  return "<tr>" + cols.map{|c| "<td>#{c}</td>"}.join(" ")  + "</tr>"
end

def ahref(name, link)
  "<a href=\"#{link}\">#{name}</a>"
end

def imgsrc(link)
  "<img src=\"#{link}\" width=\"82\">"
  #"<img src=\"#{link}\">"
end


index = File.open("index.html", "w")
index.puts "<table border=1>"
fp = File.open("scripts/mods.cfg", "r")
while (str = fp.gets)
  file, mod = str.split(/\s+/)
  dn = File.dirname(file)
  exe = "budlabel -p #{CXT} -r #{file} -i #{mod} -#{flags} -O png"
  puts "EXE #{exe}"
  IO::popen(exe) do |io|
    lines = []
    while (ln = io.gets)
      next if ln =~ /^Warning/
      puts "#{file}::#{mod}\t#{ln}"
      lines << ln
    end
    #index.puts table_row([ahref(file, file), mod, lines.join("<br>"), ahref("#{mod}.pdf", "./#{mod}.pdf"), exe])
    index.puts table_row([ahref(file, file), mod, lines.join("<br>"), imgsrc("#{mod}.png"), exe])
  end
end
fp.close
index.puts "</table>"
index.close
