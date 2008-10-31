package jol.net.servlet.util;

import java.util.List;
import java.util.Map;

import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.Table;
import jol.types.table.TableName;

public class HTML {
	final String httpBase;
	public HTML(String httpBase) {
		this.httpBase = httpBase;
	}
	public String escape(String s) {
		// XXX there is probably a real implementation of this somewhere.
		s = s.replace("<","&lt;");
		s = s.replace(">","&gt;");
		s = s.replace("&","&amp;");
		return s;
	}
	public StringBuilder header(String title) {
        StringBuilder s = new StringBuilder();
        
        s.append(
        		"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.1//EN' 'http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd'>" +
        		"<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>" +
        		"<head><meta content='text/html; charset=UTF-8' http-equiv='Content-Type'/>"
        		        );

        s.append("<title>" + title  + "</title>");
        s.append("<link href='/lincoln/lincoln.css' title='normal-page' type='text/css' rel='Stylesheet' media='screen'/>");
        s.append("</head>");
        s.append("<body>");
        s.append("<h1 class='Title'>" + title + "</h1>");

        return s;
	}
	public StringBuilder footer() {
        return  new StringBuilder("</body>\n</html>");
	}
	public StringBuilder section(StringBuilder s) {
		return section(s.toString());
	}
	public StringBuilder section(String s) {
		return new StringBuilder("<h2 class='section'>"+s+"</h2>");
	}
	public StringBuilder toString(TableName n) {
		return new StringBuilder("<a class='TableName' href='"+httpBase +n.scope+"/" +n.name+"'>"+escape(n.toString())+"</a>");
	}
	public StringBuilder toString(Key k) {
		return new StringBuilder("<span class='Key'>"+k.toString()+"</key>");
	}
	public StringBuilder toString(String s) {
		return new StringBuilder("<span class='String'>'"+s+"'</span>");
	}
	public StringBuilder toString(Class c) {
		String p = c.getCanonicalName().toString();
		String n = c.getSimpleName();
		if(p.startsWith("java.lang.")
				|| p.startsWith("jol.types.")
				|| p.startsWith("jol.lang.plan.")
				|| p.startsWith("jol.net.")) {
			return new StringBuilder("<span class='Class'>"+n+"</span>");
		} else {
			return new StringBuilder("<span class='Class'>"+p+"</span>");
		}
	}
	public StringBuilder toString(Table t) {
		return new StringBuilder("<span class='Table'>&lt;" + toString(t.name()) + "&gt;</span>");
	}
	public StringBuilder toString(TypeList t) {
		StringBuilder s = new StringBuilder("<span class='TypeList'>[");
		if(t.size() == 0) {
			s.append("<empty type list>");
		} else {
			s.append(toString((Class)t.get(0)));
			for(int i = 1; i < t.size(); i++) {
				s.append(", "+toString((Class)t.get(i)));
			}

		}
		s.append("]</span>");
		return s;
	}
	public StringBuilder toString(Object[] a) {
		StringBuilder s = new StringBuilder("<span class='Array'>[");
		if(a.length == 0) {
			s.append("empty");
		} else {
			boolean first = true;
			for(Object e : a) {
				if(!first) s.append(", ");
				first = false;
				s.append(toString(e));
			}
		}
		s.append("]</span>");
		return s;
	}
	public StringBuilder toString(Object c) {
		StringBuilder s;
		if(c instanceof Number) {
			s = new StringBuilder(escape(c.toString()));
		} else if(c instanceof String) {
			s = new StringBuilder(toString((String)c));
		} else if(c instanceof TableName) {
			s = toString((TableName)c);
		} else if(c instanceof Tuple) {
			s = toString((Tuple)c);
		} else if(c instanceof Key) {
			s = toString((Key)c);
		} else if(c instanceof TypeList) {
			s = toString((TypeList)c);
		} else if(c instanceof Table) {
			s = toString((Table) c);
		} else if(c instanceof Class) {
			s = toString((Class) c);
		} else if(c instanceof Object[]) {
			s = toString((Object[]) c);
		} else {
			s = new StringBuilder("<span class='Unknown'>&lt;" + toString((Class)c.getClass()) + ": " + escape(c.toString())+"&gt;</span>");
		}
		return s;
	}
	public StringBuilder toString(Tuple t) {
		StringBuilder s = new StringBuilder();
		s.append ("<span class='Tuple'>(");
		if(t.size() == 0) {
			s.append("<empty tuple>");
		} else {
			s.append(toString(t.value(0)));
		}
		for(int i = 1; i < t.size(); i++) {
			s.append(toString(t.value(i)));
		}
		s.append (")</span>");
		return s;
	}
	public StringBuilder toString(List<Tuple> ts) {
		StringBuilder s = new StringBuilder();
		if(ts.size() == 0) {
			s.append("<p class='LincolnTable'>[empty]</p>");
			} else {
			s.append("<table class='LincolnTable'>\n");
		
			for(Tuple tup : ts) {
			    s.append("<tr class='LincolnRow'>");
			    for(int i = 0; i < tup.size(); i++) {
					s.append("<td class='LincolnField'>");
					Comparable c = tup.value(i);
					if(c == null) {
					    s.append("null");
					} else {
							s.append(toString(tup.value(i)));
					}
				s.append("</td>");
			    }
			    s.append("</tr>\n");
			}
			s.append("</table>");
		}
		return s;
	}
	public StringBuilder httpParametersToString(Map m) {
        StringBuilder s = new StringBuilder();
        if(m.size() == 0) {
        	s.append("<p class='HttpParameters'>Empty map</p>");
        } else {
	        s.append("<ul class='HttpParameters'>");
	        for(Object n : m.keySet()) {
	        	s.append("<li>" + toString(n) + " = " + toString(m.get(n)) + "</li>" );
	        }
	        s.append("</ul>");
        }
        return s;
	}
}
