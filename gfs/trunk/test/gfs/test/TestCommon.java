package gfs.test;

import java.util.List;
import java.util.LinkedList;
import java.io.InputStream;
import java.io.File;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import junit.framework.Assert;
import gfs.Master;
import gfs.DataNode;
import gfs.Shell;
import gfs.Conf;

import org.junit.After;
import org.junit.Before;

public class TestCommon {
    protected List<Master> masters;
    protected List<DataNode> datanodes;
    protected Shell shell;

    @Before
    public void doNothing() {
    }

    @After
    public void shutdown() {
        for (Master m : this.masters) {
            m.stop();
        }
        if (this.datanodes != null) {
            for (DataNode d : this.datanodes) {
                d.stop();
            }
        }
        // shell.shutdown();
        java.lang.System.out.println("shutdown complete\n");
    }

    protected Boolean shellLs(String... list) throws JolRuntimeException,
            UpdateException, InterruptedException {
        Shell shell = new Shell();
        Boolean ret = findInLs(shell, list);
        shell.shutdown();
        return ret;
    }

    protected int shellLsCnt() throws JolRuntimeException,
            UpdateException, InterruptedException {
        Shell shell = new Shell();
        int ret = lsCnt(shell);
        shell.shutdown();
        return ret;
    }
    protected void killMaster(int index) {
        this.masters.get(index).stop();
    }


    protected void shellCreate(String name) throws JolRuntimeException, UpdateException {
        Shell shell = new Shell();
        createFile(shell, name);
        shell.shutdown();
    }

    protected void shellRm(String name) throws JolRuntimeException, UpdateException {
        Shell shell = new Shell();
        rmFile(shell, name);
        shell.shutdown();
    }

    protected void assertTrue(Boolean b) {
        /*
         * weird huh? Assert.assertTrue raises an error without calling the
         * @After method, so we never terminate.
         */
        if (!b)
            shutdown();

        Assert.assertTrue(b);
    }

    protected int lsCnt(Shell shell) throws JolRuntimeException, UpdateException {
        ValueList<String> list = lsFile(shell);
        return list.size();
    }

    protected Boolean findInLs(Shell shell, String... files) throws JolRuntimeException,
            UpdateException {
        ValueList<String> list = lsFile(shell);

        // obviously not an efficient way to do this.
        for (String item : files) {
            if (!list.contains(item))
                return false;

            java.lang.System.out.println("found " + item);
        }
        return true;
    }

    protected void stopMany() {
        for (Master sys : this.masters) {
            sys.stop();
        }
        for (DataNode d : this.datanodes) {
            d.stop();
        }
    }

    protected void startMany(String... args) throws JolRuntimeException, UpdateException {
        this.masters = new LinkedList<Master>();

        Conf.setNewMasterList(args);

        for (int i = 0; i < Conf.getNumMasters(); i++) {
            Master m = new Master(i);
            m.start();
            this.masters.add(m);
        }
    }

    protected void startManyDataNodes(String... args) throws JolRuntimeException, UpdateException {
        this.datanodes = new LinkedList<DataNode>();

        assert(args.length == Conf.getNumDataNodes());

        for (int i = 0; (i < Conf.getNumDataNodes()) && (i < args.length); i++) {
            new File(args[i]).mkdir();
            try {
                new File(args[i] + File.separator + "chunks").mkdir();
                new File(args[i] + File.separator + "checksums").mkdir();
                new File(args[i] + File.separator + "chunks/1").createNewFile();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
            DataNode d = new DataNode(i,args[i]);
            java.lang.System.out.println("new DATANODE "+ d.getPort());
            d.start();
            this.datanodes.add(d);
        }
    }


    protected ValueList<String> lsFile(Shell shell) throws UpdateException,
            JolRuntimeException {
        List<String> argList = new LinkedList<String>();
        return shell.doListFiles(argList);
    }

    protected void appendFile(Shell shell, String name, InputStream s) throws UpdateException {
        List<String> argList = new LinkedList<String>();
        argList.add(name);
        shell.doAppend(argList, s);
    }


    protected void createFile(Shell shell, String name) throws UpdateException,
            JolRuntimeException {
        List<String> argList = new LinkedList<String>();
        argList.add(name);
        shell.doCreateFile(argList, false);
    }

    protected void rmFile(Shell shell, String name) throws UpdateException,
            JolRuntimeException {
        List<String> argList = new LinkedList<String>();
        argList.add(name);
        shell.doRemove(argList);
    }
    /*
     * public static void main(String[] args) throws Exception { TestC t = new
     * GFSMMTest(); t.test1(); t.test2(); t.test3(); }
     */
}
