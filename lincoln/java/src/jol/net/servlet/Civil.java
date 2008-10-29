package jol.net.servlet;

import java.util.ArrayList;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URL;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;
import jol.types.table.Table;
import jol.types.basic.Tuple;

public class Civil extends HttpServlet {

	static int lincolnPort;
	static String lincolnProgram;
	static boolean singleton = true;
	static jol.core.Runtime runtime;

    public void doGet(HttpServletRequest request, HttpServletResponse response)
    throws IOException, ServletException
    {
    	try {
	    if(singleton) {
		synchronized (this) {
		    if(singleton) {
			lincolnPort
			    = Integer.parseInt(
				getServletConfig().getInitParameter("port"));
			lincolnProgram
			    = getServletConfig().getInitParameter("olg");

			jol.core.Runtime.ResourceLoader l
			    = jol.core.
			       Runtime.servletLoader(getServletContext());
			runtime = (jol.core.Runtime) 
			    jol.core.Runtime.create(lincolnPort, l);
			
			runtime.install("user", l.getResource(lincolnProgram));
					
			runtime.evaluate(); // Install program arguments.
			runtime.start();
		    }
	    			
		    singleton = false;
		}
	    }
    	} catch (UpdateException e) {
    		throw new ServletException(e);
    	} catch (JolRuntimeException e) {
    		throw new ServletException(e);
    	}

        String servletPath = request.getServletPath();
        String table = request.getParameter("table");
        String lastName = request.getParameter("lastname");

	String[] tok = servletPath.substring(1).split("/");
	String scope = "global";
	String name = "catalog";

	if(tok.length == 2) {
	    scope = tok[0];
	    name = tok[1];
	}

	final TableName tableName = new TableName(scope,name);

	ArrayList<Tuple> tupleList;
	try {
	    tupleList = runtime.call(
	      new jol.core.Runtime.Callback<ArrayList<Tuple>>() {
		public ArrayList<Tuple> call(jol.core.Runtime r) {

		    ArrayList<Tuple> a = new ArrayList<Tuple>();

		    for(Tuple t : r.catalog()
			    .table(tableName).primary()) {
			a.add(t);
		    }

		    return a;
		}
	    });
	} catch (NullPointerException e) {
	    throw new ServletException("Table not found", e);
	}

    	response.setContentType("text/html");
        final PrintWriter out = response.getWriter();

	out.println(
"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>" +
"<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>" +
"<head><meta content='text/html; charset=UTF-8' http-equiv='Content-Type'/>"
);

        out.println("<title>Linoln test interface</title>");
        out.println("</head>");
        out.println("<body>");
	out.println("<h1>Running " + lincolnProgram + " on port " + lincolnPort +"</h1>");
	out.println("<h2>"+tableName+"</h2>");
	out.println("<table>");

	for(Tuple tup : tupleList) {
	    out.print("<tr>");
	    for(int i = 0; i < tup.size(); i++) {
		out.print("<td>");
		Comparable c = tup.value(i);
		String val;
		if(c == null) {
		    val = "null";
		} else {
		    if(tup.value(i) instanceof TableName) {
			TableName tn = (TableName)tup.value(i);
			val = "<a href='/lincoln/"+tn.scope+"/"+tn.name+"'>"
			    + tn.toString() + "</a>";
		    } else {
			val = tup.value(i).toString();
	// XXX there is probably a real implementaion of this somewhere.
			val = val.replace("<","&lt;");
			val = val.replace(">","&gt;");
			val = val.replace("&","&amp;");
		    }
		}
		out.print(val);
		out.print("</td>");
	    }
	    out.println("</tr>");
	}
	out.println("</table>");
	/*	out.println("<p>");
        out.println("Parameters in this request:</p>");
        out.println("<p>Servlet path: " + servletPath + "</p>");

        if (table != null || lastName != null) {
            out.println("<p>Dump table: " + table + "</p>");
            out.println("<p>Last Name: " + lastName + "</p>");
        } else {
            out.println("<p>No Parameters, Please enter some</p>");
        }
        out.print("<form action='' ");
        out.println("method='post'>");
        out.println("<p>");
        out.println("Table:");
        out.println("<input type='text' size='20' name='firstname'/>");
        out.println("<br/>");
        out.println("Last Name:");
        out.println("<input type='text' size='20' name='lastname'/>");
        out.println("<br/>");
        out.println("<input type='submit'/>");
	out.println("</p>"); */
        out.println("</form>");
        out.println("</body>");
        out.println("</html>");
    }

    public void doPost(HttpServletRequest request,
		       HttpServletResponse response)
    throws IOException, ServletException
    {
        doGet(request, response);
    }
}