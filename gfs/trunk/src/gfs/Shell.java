package gfs;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.SynchronousQueue;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

public class Shell {
	private static System system;
	private static int id = 1;

    /*
     * TODO:
     *  (1) connect to an instance of JOL
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into JOL; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        List<String> argList = Arrays.asList(args);

        if (argList.size() == 0)
            usage();

        system = Runtime.create(12345);
        system.install("gfs", ClassLoader.getSystemResource("gfs/gfs_client.olg"));
        system.install("gfs", ClassLoader.getSystemResource("gfs/gfs_master.olg"));

        String op = argList.remove(0);

        if (op.equals("cat"))
            doConcatenate(argList);
        else if (op.equals("mkdir"))
            doCreateDir(argList);
        else if (op.equals("create"))
            doCreateFile(argList);
        else
            usage();
    }

    /*
     * XXX: consider parallel evaluation
     */
    private static void doConcatenate(List<String> args) throws UpdateException
    {
        if (args.size() == 0)
            usage();
        
        for (String file : args)
        	doCatFile(file);
    }
    
    private static void doCatFile(String file) throws UpdateException
    {
    	final int request_id = generateId();
    	final SynchronousQueue<String> content_queue = new SynchronousQueue<String>();
    	
    	// Create the request tuple
        TableName tblName = new TableName("gfs", "cat_requests");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(request_id, file));
        
        // Register callback to listen for responses
        Callback response_callback = new Callback() {
			@Override
			public void deletion(TupleSet tuples) {
				;
			}

			@Override
			public void insertion(TupleSet tuples) {
				for (Tuple t : tuples)
				{
					if (t.value("request_id").equals(request_id))
					{
						String response_contents = (String) t.value("contents");
						content_queue.add(response_contents);
					}
				}
			}
        };
        Table responseTbl = system.catalog().table(tblName);
        responseTbl.register(response_callback);
        
        // Do the insert
        system.schedule("gfs", tblName, req, null);
        
        // Wait for the response
        String contents = content_queue.remove();
        responseTbl.unregister(response_callback);
        
        java.lang.System.out.println("File name: " + file);
        java.lang.System.out.println("Contents:");
        java.lang.System.out.println(contents);
        java.lang.System.out.println("=============");
    }

    private static int generateId() {
    	return id++;
	}

	private static void doCreateDir(List<String> args)
    {
        ;
    }

    private static void doCreateFile(List<String> args)
    {
        ;
    }

    private static void usage() {
        java.lang.System.err.println("Usage: java gfs.Shell op_name args");
        java.lang.System.err.println("Where op_name = {cat,mkdir,write}");
        java.lang.System.exit(0);
    }
}
