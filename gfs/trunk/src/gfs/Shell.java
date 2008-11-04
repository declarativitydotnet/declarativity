package gfs;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.SynchronousQueue;
import java.util.Random;

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
    private System system;
    private int nextId = 1;
    private String selfAddress;
    private Random rand;

    /*
     * TODO:
     *  (1) connect to an instance of JOL
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into JOL; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) throws Exception {
        List<String> argList = new LinkedList<String>(Arrays.asList(args));

        if (argList.size() == 0)
            usage();

        Shell shell = new Shell();

        String op = argList.remove(0);
        if (op.equals("cat"))
            shell.doConcatenate(argList);
        else if (op.equals("create"))
            shell.doCreateFile(argList);
        else if (op.equals("ls"))
            shell.doListDirs(argList);
        else if (op.equals("mkdir"))
            shell.doCreateDir(argList);
        else
            usage();

        shell.shutdown();
    }

    Shell() throws JolRuntimeException, UpdateException {
        this.rand = new Random();
        this.system = Runtime.create(5501);

        this.system.catalog().register(new MasterRequestTable((Runtime) this.system));
        this.system.catalog().register(new SelfTable((Runtime) this.system));

        this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
        this.system.evaluate();

        /* Identify which address the local node is at */
        this.selfAddress = "tcp:localhost:5501";
        TupleSet self = new TupleSet();
        self.add(new Tuple(this.selfAddress));
        this.system.schedule("gfs", SelfTable.TABLENAME, self, null);
        this.system.evaluate();
        this.system.start();
    }

    /*
     * XXX: consider parallel evaluation
     */
    private void doConcatenate(List<String> args) throws UpdateException, InterruptedException, JolRuntimeException {
        if (args.size() == 0)
            usage();

        for (String file : args)
            doCatFile(file);
    }

    private void doCatFile(String file) throws UpdateException, InterruptedException, JolRuntimeException {
        final int requestId = generateId();
        final SynchronousQueue<String> content_queue = new SynchronousQueue<String>();

        // Register callback to listen for responses
        Callback response_callback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {
                ;
            }

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        String responseContents = (String) t.value(2);
                        try {
                            content_queue.put(responseContents);
                            break;
                        }
                        catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                    }
                }
            }
        };
        Table responseTbl = this.system.catalog().table(new TableName("gfs", "cat_response"));
        responseTbl.register(response_callback);

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "cat_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(this.selfAddress, requestId, file));
        this.system.schedule("gfs", tblName, req, null);
        this.system.evaluate();

        // Wait for the response
        String contents = content_queue.take();
        responseTbl.unregister(response_callback);

        java.lang.System.out.println("File name: " + file);
        java.lang.System.out.println("Contents: " + contents);
        java.lang.System.out.println("=============");
    }

    private int generateId() {
        return rand.nextInt();
        //        return nextId++;
    }

    private void doCreateDir(List<String> args) {
        ;
    }

    private void doCreateFile(List<String> args) {
        ;
    }

    private void doListDirs(List<String> args) {
        if (args.size() == 0)
            usage();

        for (String dir : args)
            doListDir(dir);
    }

    private void doListDir(String dirname) {
        ;
    }

    private void shutdown() {
        this.system.shutdown();
    }

    private static void usage() {
        java.lang.System.err.println("Usage: java gfs.Shell op_name args");
        java.lang.System.err.println("Where op_name = {cat,create,ls,mkdir}");
        java.lang.System.exit(0);
    }
}
