package gfs;

import java.util.Arrays;
import java.util.List;

import jol.core.Runtime;
import jol.core.System;
import jol.types.exception.P2RuntimeException;

public class Shell {
	private static System system;

    /*
     * TODO:
     *  (1) connect to an instance of P2
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into P2; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) throws P2RuntimeException {
        List<String> argList = Arrays.asList(args);

        if (argList.size() == 0)
            usage();

        system = Runtime.create(123);

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

    private static void doConcatenate(List<String> args)
    {
        if (args.size() == 0)
            usage();
        
        /*
         * TODO:
         * (1) For each file to list, add an entry into the "outgoing concat requests" table
         * (2) Register a callback on the "concat responses" table to listen for responses
         */
        for (String file : args) {
        	;
        }
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
