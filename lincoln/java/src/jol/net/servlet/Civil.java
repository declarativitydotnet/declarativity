package jol.net.servlet;

import java.util.ArrayList;
import java.util.Collection;
import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import jol.core.Runtime;
import jol.core.Runtime.RuntimeCallback;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;

public class Civil extends LincolnServlet {

	private void printTable(PrintWriter out, Collection<Tuple> ts) {
        out.println("<table>");

    	for(Tuple tup : ts) {
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

	}
	
    @Override
	public void doGet(HttpServletRequest request, HttpServletResponse response)
    throws IOException, ServletException
    {
    	jol.core.Runtime runtime = getLincolnRuntime();

        String servletPath = request.getServletPath();
        
        String[] tok = servletPath.substring(1).split("/");
        String scope = "global";
        String name = "catalog";

        if(tok.length == 2) {
        	scope = tok[0];
        	name = tok[1];
        }

        final TableName tableName = new TableName(scope,name);

        final RuntimeCallback<ArrayList<Tuple>> dumpTable = 
        	new RuntimeCallback<ArrayList<Tuple>>() {
				public ArrayList<Tuple> call(Runtime r) {

					ArrayList<Tuple> a = new ArrayList<Tuple>();

					for(Tuple t : r.catalog().table(tableName).primary()) {
						a.add(t);
					}
					return a;
				}
			};

		final TupleSet insertions = new TupleSet();
		final TupleSet deletions = new TupleSet();

		String action = request.getParameter("Action");
		
		if(action != null) {
			Tuple t = new Tuple(
					(String)request.getParameter("from"),
					(String)request.getParameter("to")
					);
			// XXX this bypasses the type checker.  If we insert the wrong type of tuple, 
			// then bad things happen!
			if(action.equals("Insert")) {
				insertions.add(t);
			} else if (action.equals("Delete")) {
				deletions.add(t);
			}
		}

		
		final String prog = "path";
		final TableName linkTable= new TableName("path", "link");
		final TableName shortestPathTable = new TableName("path", "shortestPath");
		
		final ArrayList<Tuple> deltaAdd = new ArrayList<Tuple>();
		final ArrayList<Tuple> deltaErase = new ArrayList<Tuple>();

		final jol.core.Runtime.RuntimeCallback<Boolean> inject =
			new jol.core.Runtime.RuntimeCallback<Boolean>() {

				@Override
				public Boolean call(Runtime r) throws UpdateException, JolRuntimeException {
					
					Table sp = r.catalog().table(shortestPathTable);
					Callback logger = new Callback() {

						@Override
						public void deletion(TupleSet tuples) {
							deltaErase.addAll(tuples);
						}

						@Override
						public void insertion(TupleSet tuples) {
							deltaAdd.addAll(tuples);
						}
					};

					sp.register(logger);
					
					r.schedule(prog, linkTable, insertions, deletions);
					r.evaluateFixpoint();
				
					sp.unregister(logger);
					return false;
				}
				
			};
			
		jol.core.Runtime.RuntimeCallback<Object[]> allAtOnce =
			new jol.core.Runtime.RuntimeCallback<Object[]>() {

				@Override
				public Object[] call(Runtime r) throws UpdateException, JolRuntimeException {
					return new Object[] {
							dumpTable.call(r),
							inject.call(r),
							dumpTable.call(r)
					};
				}
				
			};
        Collection<Tuple> tupleList, tupleList2;
        try {
        	Object[] all = runtime.call(allAtOnce);
        	tupleList = (Collection<Tuple>) all[0];
        	tupleList2 = (Collection<Tuple>) all[2];
        } catch (UpdateException e) {
        	throw new ServletException("Table not found", e);
        } catch (JolRuntimeException e) {
        	throw new ServletException("Table not found", e);
        }

    	response.setContentType("text/html");
        final PrintWriter out = response.getWriter();

        out.println(
"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>" +
"<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>" +
"<head><meta content='text/html; charset=UTF-8' http-equiv='Content-Type'/>"
        );

        out.println("<title>" + getLincolnProgramName() + " on port " + getLincolnPort()  + "</title>");
        out.println("</head>");
        out.println("<body>");
        out.println("<h1>Running " + getLincolnProgramName() + " on port " + getLincolnPort() +"</h1>");
        out.println("<h2>"+tableName+"</h2>");

        out.print("<form action='' method='post'>");
        out.println("<p>New link: ");
        out.println("<input type='text' size='20' name='from'/>");
        out.println(" -> ");
        out.println("<input type='text' size='20' name='to'/>");
        out.println(" ");
        out.println("<input type='submit' name='Action' value='Insert'/>");
        out.println("<input type='submit' name='Action' value='Delete'/>");
        out.println("</p>"); 
        out.println("</form>");

        out.print("<ul>");
        for(Object n : request.getParameterMap().keySet()) {
        	out.print("<li>'" + n + "' = '" + request.getParameter((String)n) + "'</li>" );
        }
        out.print("</ul>");

        out.print("<h2>Before</h2>");
        
        printTable(out, tupleList);

        out.println("<h2>path:shortestPath deletions</h2>");

        printTable(out, deltaErase);
        
        out.println("<h2>path::shortestPath insertions</h2>");
        
        printTable(out, deltaAdd);
        
        out.print("<h2>After</h2>");
        
        printTable(out, tupleList2);
        
        
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
	out.println("</p>"); 
        out.println("</form>");
        */
        out.println("</body>");
        out.println("</html>");
    }

}