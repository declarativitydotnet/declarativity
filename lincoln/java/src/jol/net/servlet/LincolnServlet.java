package jol.net.servlet;

import java.io.IOException;
import java.util.HashMap;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;

public class LincolnServlet extends HttpServlet {

	private static HashMap<String, jol.core.Runtime> lincolns = new HashMap<String, jol.core.Runtime>();
	
	public int getLincolnPort() {
		return  Integer.parseInt(getServletConfig().getInitParameter("port"));
	}
	public String getLincolnProgramName() {
		return getServletConfig().getInitParameter("olg");
	}
	@Override
	public void init() throws ServletException {
		super.init();
		getLincolnRuntime();
	}
	public jol.core.Runtime getLincolnRuntime() throws ServletException {
		synchronized(lincolns) {
			try {
				String lincolnProgram = getLincolnProgramName()+":"+getLincolnPort();
				jol.core.Runtime ret = lincolns.get(lincolnProgram);
				if(ret == null) {
	
					jol.core.Runtime.ResourceLoader l
					    = jol.core.Runtime.servletLoader(getServletContext());
					ret = (jol.core.Runtime) jol.core.Runtime.create(getLincolnPort(), l);
					ret.install("user", l.getResource(getLincolnProgramName()));
					ret.evaluate(); // Install program arguments.
					ret.start();
					lincolns.put(lincolnProgram, ret);
				}
				return ret;
			} catch (JolRuntimeException e) {
				throw new ServletException(e);
			} catch (UpdateException e) {
				throw new ServletException(e);
			} 
		}
	}
    @Override
	public void doPost(HttpServletRequest request,
		       HttpServletResponse response)
    throws IOException, ServletException
    {
        doGet(request, response);
    }
}
