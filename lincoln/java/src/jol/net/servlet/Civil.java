package jol.net.servlet;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import jol.core.Runtime;
import jol.core.Runtime.RuntimeCallback;
import jol.net.servlet.util.HTML;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

public class Civil extends LincolnServlet {

    @Override
	public void doGet(HttpServletRequest request, HttpServletResponse response)
    throws IOException, ServletException
    {
    	jol.core.Runtime runtime = getLincolnRuntime();
    	
    	String urlprefix = request.getContextPath()+request.getServletPath()+"/";
        String subdir = request.getPathInfo();
        
        String scope = "global";
        String name = "catalog";

        if(subdir != null && subdir.length() > 0) {
        
	        String[] tok = subdir.substring(1).split("/");
	        
	        if(tok.length == 2) {
	        	scope = tok[0];
	        	name = tok[1];
	        }
        }
        final TableName tableName = new TableName(scope,name);

        final ArrayList<Tuple> preTable = new ArrayList<Tuple>();
        final ArrayList<Tuple> postTable = new ArrayList<Tuple>();
        
        final RuntimeCallback dumpTablePre = 
        	new RuntimeCallback() {
      	
				public void call(Runtime r) {
					for(Tuple t : r.catalog().table(tableName).primary()) {
						preTable.add(t);
					}
				}
			};
        final RuntimeCallback dumpTablePost = 
        	new RuntimeCallback() {
      	
				public void call(Runtime r) {
					for(Tuple t : r.catalog().table(tableName).primary()) {
						postTable.add(t);
					}
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

		final Table sp = runtime.catalog().table(shortestPathTable);
		final Callback logger = new Callback() {

			@Override
			public void deletion(TupleSet tuples) {
				deltaErase.addAll(tuples);
			}

			@Override
			public void insertion(TupleSet tuples) {
				deltaAdd.addAll(tuples);
			}
		};

		final RuntimeCallback pre = new RuntimeCallback() {
				@Override
				public void call(Runtime r) throws UpdateException, JolRuntimeException {
					sp.register(logger);
					r.schedule(prog, linkTable, insertions, deletions);
				}
			};

		final RuntimeCallback post = new RuntimeCallback() {

			@Override
			public void call(Runtime r) throws UpdateException,
					JolRuntimeException {
					sp.unregister(logger);
			}
			
		};
			
      try {
        	runtime.evaluateFixpoint(
        			Runtime.callbacks(dumpTablePre,pre),
        			Runtime.callbacks(post,dumpTablePost)
        	);
        } catch (UpdateException e) {
        	throw new ServletException("Table not found", e);
        } catch (JolRuntimeException e) {
        	throw new ServletException("Table not found", e);
        }

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
        out.print(format.toString(preTable));
        
        out.println(format.section(format.toString(shortestPathTable) + " deletions"));
        out.print(format.toString(deltaErase));
        
        out.println(format.section(format.toString(shortestPathTable) + " insertions"));
        out.print(format.toString(deltaAdd));
        
        out.print(format.section("Before"));
        out.print(format.toString(postTable));
        
        out.print(format.footer());
        
    }

}