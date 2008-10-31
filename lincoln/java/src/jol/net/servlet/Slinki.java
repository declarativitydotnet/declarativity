package jol.net.servlet;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import jol.core.Runtime.RuntimeCallback;
import jol.net.servlet.util.Lincoln;
import jol.net.servlet.util.Lincoln.DeltaLogger;
import jol.net.servlet.util.Lincoln.InjectTuples;
import jol.net.servlet.util.Lincoln.Logger;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;

public class Slinki extends LincolnServlet {

   @Override
	public void doGet(HttpServletRequest request, HttpServletResponse response)
    throws IOException, ServletException
    {
    	jol.core.Runtime runtime = getLincolnRuntime();
    	
    	String urlprefix = getRequestRoot(request);
        String subdir = getRequestSubdirectory(request);

        String name = subdir;
        if(name == null) {
        	name = "Main";
        }
        if(name.startsWith("/")) { name = name.substring(1); }
        if(name.endsWith("/")) { name = name.substring(0,name.length()-1); }

        String setParam = request.getParameter("content"); 
        if(setParam != null) {
        	InjectTuples create = new InjectTuples("slinki",
        										   new TableName("slinki", "setPage"));
        	create.insertions().add(new Tuple(name, setParam));

	        try {
				Lincoln.evaluateTimestep(runtime,
	    			  		   	new RuntimeCallback[] { create },
	    			  		   	new Logger[] 		  {        },
	    			  		   	new RuntimeCallback[] {        });
	        } catch (UpdateException e) {
	        	throw new ServletException(e);
	        } catch (JolRuntimeException e) {
	        	throw new ServletException(e);
	        }
	        response.sendRedirect(urlprefix + subdir);
        	
        } else {

        	InjectTuples query;
        	String desiredVersion = request.getParameter("version"); 
        	if(desiredVersion != null) { 
        		query = new InjectTuples("slinki", new TableName("slinki", "getPageVersion"));
    	        query.insertions().add(new Tuple(name, Integer.parseInt(desiredVersion)));
        	} else{ 
        		query = new InjectTuples("slinki", new TableName("slinki","getPage"));
    	        query.insertions().add(new Tuple(name));

        	}
	        DeltaLogger result = new DeltaLogger(runtime, new TableName("slinki", "getPageResult"));
	        
	        
	        try {
				Lincoln.evaluateTimestep(runtime,
	    			  		   	new RuntimeCallback[] { query  },
	    			  		   	new Logger[] 		  { result },
	    			  		   	new RuntimeCallback[] {        });
	        } catch (UpdateException e) {
	        	e.printStackTrace(); //throw new ServletException(e);
	        } catch (JolRuntimeException e) {
	        	e.printStackTrace(); throw new ServletException(e);
	        }
	        String page;
	        Integer version = 0;
	        if(result.getInsertions().size() == 1) {
	        	page = (String)result.getInsertions().get(0).value(0);
	        	version = (Integer)result.getInsertions().get(0).value(1);
	        } else {
	        	page = "= " + name + "\nClick edit (on the right) to create this new page."; 
	        }
	        

	        BufferedReader template = 
	        	new BufferedReader(
	        			new InputStreamReader(
	        					getServletContext()
	        					    .getResourceAsStream(
	        					    		"/slinki-site/creole-template.html")));

	        
	        String in;
	        response.setContentType("text/html");
	        PrintWriter out = response.getWriter();

	        while((in = template.readLine()) != null) {
	        	out.println(in.replaceAll("__TITLE__",name)
	        				.replaceAll("__CONTENT__",page)
	        				.replaceAll("__MAXVERSION__",version.toString()));
	        }
        }
    }
}
