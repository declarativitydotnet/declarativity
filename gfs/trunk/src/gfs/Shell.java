package gfs;

import java.util.Arrays;
import java.util.List;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

public class Shell {
	private static System system;

    /*
     * TODO:
     *  (1) connect to an instance of P2
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into P2; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) throws P2RuntimeException, UpdateException {
        List<String> argList = Arrays.asList(args);

        if (argList.size() == 0)
            usage();

        system = Runtime.create(123);
        system.install("gfs", ClassLoader.getSystemResource("nrc_gfs.olg"));

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

    private static void doConcatenate(List<String> args) throws UpdateException
    {
        if (args.size() == 0)
            usage();

        TableName tblName = new TableName("gfs", "cat_requests");
        // Register callback to listen for responses
        Table responseTbl = system.catalog().table(tblName);
        responseTbl.register(new Callback(){
			@Override
			public void deletion(TupleSet tuples) {
				;
			}

			@Override
			public void insertion(TupleSet tuples) {
				;
			}        	
        });
        /*
         * TODO:
         * (1) For each file to list, add an entry into the "outgoing requests" table
         *   => How do we identify an outgoing request, such that it is (a) distinct
         *      from any other concurrent requests in the system (b) in the callback,
         *      we can identify which response tuple is for which request
         *   
         * (2) Register a callback on the "concat responses" table to listen for responses
         */
        TupleSet reqs = new TupleSet(tblName);
        for (String file : args) {
        	reqs.add(new Tuple(1, file));
        }
        system.schedule("gfs", tblName, reqs, null);
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
        java.lang.System.out.println("Usage: java gfs.Shell op_name args");
        java.lang.System.out.println("Where op_name = {cat,mkdir,write}");
        java.lang.System.exit(0);
    }
}
