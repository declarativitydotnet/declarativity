package gfs;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.SynchronousQueue;
import java.util.Random;

import jol.core.Runtime;
import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

public class Shell {
    private System system;
    private String selfAddress;
    private Random rand;
    private SynchronousQueue responseQueue;

    /*
     * TODO:
     *  (1) connect to an instance of JOL
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into JOL; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) throws Exception {
        Shell shell = new Shell();
        List<String> argList = new LinkedList<String>(Arrays.asList(args));

        if (argList.size() == 0)
            shell.usage();

        String op = argList.remove(0);
        if (op.equals("cat"))
            shell.doConcatenate(argList);
        else if (op.equals("create"))
            shell.doCreateFile(argList);
        else if (op.equals("ls"))
            shell.doListFiles(argList);
        else if (op.equals("rm"))
            shell.doRemove(argList);
        else
            shell.usage();

        shell.shutdown();
    }

    Shell() throws JolRuntimeException, UpdateException {
        this.rand = new Random();
        this.responseQueue = new SynchronousQueue();

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
    private void doConcatenate(List<String> args) throws UpdateException, InterruptedException {
        if (args.isEmpty())
            usage();

        for (String file : args)
            doCatFile(file);
    }

    private void doCatFile(final String file) throws UpdateException, InterruptedException {
        final int requestId = generateId();

        // Register callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(2);
                        Object content = t.value(3);

                        if (success.booleanValue()) {
                            java.lang.System.out.println("File name: " + file);
                            java.lang.System.out.println("Content: " + content);
                            java.lang.System.out.println("=============");
                        } else {
                            java.lang.System.out.println("ERROR on \"cat\":");
                            java.lang.System.out.println("File name: " + file);
                            java.lang.System.out.println("Error message: " + content);
                        }

                        try {
                            responseQueue.put(content);
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
        responseTbl.register(responseCallback);

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "cat_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(this.selfAddress, requestId, file));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        Object obj = this.responseQueue.take();
        responseTbl.unregister(responseCallback);
    }

    private int generateId() {
        return rand.nextInt();
    }

    private void doCreateFile(List<String> args) throws UpdateException, InterruptedException {
        if (args.size() != 1)
            usage();

        /* Read the contents of the file from stdin */
        StringBuilder sb = new StringBuilder();
        int b;
        try {
            while ((b = java.lang.System.in.read()) != -1)
                sb.append((char) b);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        String filename = args.get(0);
        final int requestId = generateId();

        // Register a callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(2);

                        if (success.booleanValue()) {
                            java.lang.System.out.println("Create succeeded.");
                        } else {
                            String errMessage = (String) t.value(3);
                            java.lang.System.out.println("Create failed.");
                            java.lang.System.out.println("Error message: " + errMessage);
                        }

                        try {
                            responseQueue.put(success);
                            break;
                        } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                    }
                }
            }
        };

        Table responseTbl = this.system.catalog().table(new TableName("gfs", "create_response"));
        responseTbl.register(responseCallback);

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "create_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(this.selfAddress, requestId, filename, sb.toString()));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        Object obj = this.responseQueue.take();
        responseTbl.unregister(responseCallback);
    }

    private void doListFiles(List<String> args) throws UpdateException, InterruptedException {
        if (!args.isEmpty())
            usage();

        final int requestId = generateId();

        // Register a callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Object lsContent = t.value(2);
                        try {
                            responseQueue.put(lsContent);
                            break;
                        } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                    }
                }
            }
        };

        Table responseTbl = this.system.catalog().table(new TableName("gfs", "ls_response"));
        responseTbl.register(responseCallback);

        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "ls_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(this.selfAddress, requestId));
        this.system.schedule("gfs", tblName, req, null);

        ValueList<String> lsContent = (ValueList<String>) this.responseQueue.take();
        Collections.sort(lsContent);
        responseTbl.unregister(responseCallback);

        java.lang.System.out.println("ls:");
        int i = 1;
        for (String file : lsContent) {
            java.lang.System.out.println("  " + i + ". " + file);
            i++;
        }
    }

    private void doRemove(List<String> argList) throws UpdateException, InterruptedException {
        if (argList.isEmpty())
            usage();
        
        for (String s : argList) {
            doRemoveFile(s);
        }
    }

    private void doRemoveFile(final String file) throws UpdateException, InterruptedException {
        final int requestId = generateId();

        // Register callback to listen for responses
        Callback responseCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    Integer tupRequestId = (Integer) t.value(1);

                    if (tupRequestId.intValue() == requestId) {
                        Boolean success = (Boolean) t.value(2);

                        if (success.booleanValue()) {
                            java.lang.System.out.println("Remove of file \"" + file + "\": success.");
                        } else {
                            Object content = t.value(3);
                            java.lang.System.out.println("ERROR on \"rm\":");
                            java.lang.System.out.println("File name: " + file);
                            java.lang.System.out.println("Error message: " + content);
                        }

                        try {
                            responseQueue.put(success);
                            break;
                        }
                        catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                    }
                }
            }
        };
        Table responseTbl = this.system.catalog().table(new TableName("gfs", "rm_response"));
        responseTbl.register(responseCallback);
        
        // Create and insert the request tuple
        TableName tblName = new TableName("gfs", "rm_request");
        TupleSet req = new TupleSet(tblName);
        req.add(new Tuple(this.selfAddress, requestId, file));
        this.system.schedule("gfs", tblName, req, null);

        // Wait for the response
        Object obj = this.responseQueue.take();
        responseTbl.unregister(responseCallback);
    }

    private void shutdown() {
        this.system.shutdown();
    }

    private void usage() {
        java.lang.System.err.println("Usage: java gfs.Shell op_name args");
        java.lang.System.err.println("Where op_name = {cat,create,ls,rm}");

        shutdown();
        java.lang.System.exit(0);
    }
}
