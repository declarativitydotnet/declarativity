package jol.net.servlet;

import java.io.IOException;
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import jol.core.Runtime.RuntimeCallback;
import jol.net.servlet.util.HTML;
import jol.net.servlet.util.Lincoln;
import jol.net.servlet.util.Lincoln.DeltaLogger;
import jol.net.servlet.util.Lincoln.DumpTable;
import jol.net.servlet.util.Lincoln.InjectTuples;
import jol.net.servlet.util.Lincoln.Logger;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;

public class Civil extends LincolnServlet {
    private static final long serialVersionUID = 1L;

    @Override
	public void doGet(HttpServletRequest request, HttpServletResponse response)
    throws IOException, ServletException
    {
    	jol.core.Runtime runtime = getLincolnRuntime();
    	
    	String urlprefix = getRequestRoot(request);
        String subdir = getRequestSubdirectory(request);

        // ------ Decide which table to display the contents of
        
        String scope = "global";
        String name = "catalog";

        if(subdir != null && subdir.length() > 0) {
        
	        String[] tok = subdir.substring(1).split("/");
	        
	        if(tok.length == 2) {
	        	scope = tok[0];
	        	name = tok[1];
	        }
        }
        
        TableName tableName = new TableName(scope,name);
        DumpTable preTableDump  = new DumpTable(tableName);
        DumpTable postTableDump = new DumpTable(tableName);

        // ------ Setup callbacks for injecting / deleting tuple
        
		InjectTuples inject = new InjectTuples("path", new TableName("path", "link"));
		String action = request.getParameter("Action");

		if(action != null) {
			Tuple t = new Tuple(
					(String)request.getParameter("from"),
					(String)request.getParameter("to")
					);
			// XXX this bypasses the type checker.  If we insert the wrong type of tuple, 
			// then bad things happen!
			if(action.equals("Insert")) {
				inject.insertions().add(t);
			} else if (action.equals("Delete")) {
				inject.deletions().add(t);
			}
		}

		// ------ Setup callback to log the differences to the shortestPath table.
		
		TableName shortestPathTable = new TableName("path", "shortestPath");
		DeltaLogger logger;

		try {
			logger = new DeltaLogger(runtime, shortestPathTable);
		} catch (Exception e) {
			logger = null;
		}

		// ------ Run the lincoln query
			
		try {
			if(logger != null) {
				Lincoln.evaluateTimestep(runtime,
	    			  		   	new RuntimeCallback[] { preTableDump, inject },
	    			  		   	new Logger[] 		  { logger               },
	    			  		   	new RuntimeCallback[] { postTableDump        });
			} else {
				Lincoln.evaluateTimestep(runtime,
			  		   	new RuntimeCallback[] { preTableDump, },
			  		   	new Logger[] 		  { },
			  		   	new RuntimeCallback[] { });
			}
		} catch (UpdateException e) {
			throw new ServletException("Table not found", e);
		} catch (JolRuntimeException e) {
			throw new ServletException("Table not found", e);
		}

		// ------ Render the html response
		
		response.setContentType("text/html");

    	HTML format = new HTML(urlprefix);
    	
    	final PrintWriter out = response.getWriter();

    	String title = getLincolnProgramName() + " on port " + getLincolnPort();
        out.print(format.header(title));

        out.print(format.section(format.toString(tableName)));
        
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

        out.print(format.httpParametersToString(request.getParameterMap()));

        out.print(format.section("Before"));
        out.print(format.toString(preTableDump.result()));
        
        if(logger != null) {
        
	        out.println(format.section(format.toString(shortestPathTable) + " deletions"));
	        out.print(format.toString(logger.getDeletions()));
	        
	        out.println(format.section(format.toString(shortestPathTable) + " insertions"));
	        out.print(format.toString(logger.getInsertions()));
	        
	        out.print(format.section("After"));
	        out.print(format.toString(postTableDump.result()));
        }
        
        out.print(format.footer());
        
    }

}