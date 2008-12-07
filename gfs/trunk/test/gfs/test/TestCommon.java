package gfs.test;

import java.util.List;
import java.util.LinkedList;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import junit.framework.Assert;
import gfs.Master;
import gfs.Shell;
import gfs.Conf;

import org.junit.After;
import org.junit.Before;

public class TestCommon {
    protected ValueList<Master> masters;
    protected Shell shell;

    @Before
    public void doNothing() {
    }

    @After
    public void shutdown() {
        for (Master m : masters) {
            m.stop();
        }
        // shell.shutdown();
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


    protected void shellCreate(String name) throws JolRuntimeException, UpdateException,
            InterruptedException {
        Shell shell = new Shell();
        createFile(shell, name);
        shell.shutdown();
    }

    protected void shellRm(String name) throws JolRuntimeException, UpdateException,
            InterruptedException {
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

    protected int lsCnt(Shell shell) throws JolRuntimeException, UpdateException, InterruptedException {
        ValueList<String> list = lsFile(shell);
        return list.size();
    }

    protected Boolean findInLs(Shell shell, String... files) throws JolRuntimeException,
            UpdateException, InterruptedException {
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
    }

    protected void startMany(String... args) throws JolRuntimeException, UpdateException {
        this.masters = new ValueList<Master>();

        Conf.setNewMasterList(args);

        for (int i = 0; i < Conf.getNumMasters(); i++) {
            Master m = new Master(i);
            m.start();
            this.masters.add(m);
        }
    }

    protected ValueList<String> lsFile(Shell shell) throws UpdateException,
            InterruptedException, JolRuntimeException {
        List<String> argList = new LinkedList<String>();
        return shell.doListFiles(argList);
    }

    protected void createFile(Shell shell, String name) throws UpdateException,
            InterruptedException, JolRuntimeException {
        List<String> argList = new LinkedList<String>();
        argList.add(name);
        shell.doCreateFile(argList, false);
    }

    protected void rmFile(Shell shell, String name) throws UpdateException,
            InterruptedException, JolRuntimeException {
        List<String> argList = new LinkedList<String>();
        argList.add(name);
        shell.doRemove(argList);
    }
    /*
     * public static void main(String[] args) throws Exception { TestC t = new
     * GFSMMTest(); t.test1(); t.test2(); t.test3(); }
     */
}
