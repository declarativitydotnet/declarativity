package jol.test;

import java.io.FileWriter;
import java.net.URL;
import java.util.Random;

import jol.core.Runtime;
import jol.core.JolSystem;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.Table;
import jol.types.table.TableName;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class Tester {
	public static String[][] TESTS = {
		{"jol/test/aggregationTest1.olg", "test::checkTable"}
	};

	public static enum CheckField{CHECK, SOLUTION};

    private JolSystem sys;

    private URL programFile;

    private TableName checkTableName;

    public Tester(URL programFile, TableName checkTableName) {
    	this.programFile = programFile;
    	this.checkTableName = checkTableName;
    }

    @Before
    public void setup() throws JolRuntimeException, UpdateException {
        this.sys = Runtime.create(true, 5000);
        this.sys.install("test", this.programFile);
        this.sys.evaluate();    // XXX: temporary workaround for runtime bug
    }

    @After
    public void shutdown() {
        System.err.println("shutdown() invoked");
        this.sys.shutdown();
    }

    @Test
    public void test() throws JolRuntimeException, BadKeyException {
    	this.sys.evaluate();

    	Table checkTable = this.sys.catalog().table(this.checkTableName);

    	for (Tuple check : checkTable.tuples()) {
    		TableName checkName    = (TableName) check.value(CheckField.CHECK.ordinal());
    		TableName solutionName = (TableName) check.value(CheckField.SOLUTION.ordinal());

    		Table testTable     = this.sys.catalog().table(checkName);
    		Table solutionTable = this.sys.catalog().table(solutionName);

    		Assert.assertTrue(testTable.cardinality() == solutionTable.cardinality());

    		Key key = solutionTable.primary().key();
    		for (Tuple testTuple : testTable.tuples()) {
    			Tuple lookup = key.project(testTuple);
    			TupleSet lookupResult   = testTable.primary().lookupByKey(lookup);
    			TupleSet lookupSolution = solutionTable.primary().lookupByKey(lookup);
    			Assert.assertTrue("LOOKUP FAILURE: RESULT " + lookupResult +
    					          " != SOLUTION " + lookupSolution,
    					          lookupResult.size() == 1 &&
    					          lookupResult.size() == lookupSolution.size());
    		}
    	}
    }

    public static void main(String[] args) throws Exception {
    	Random rand = new Random();
    	FileWriter writer = new FileWriter("test/data/data1.txt");
    	for (int i = 0; i < 5000; i++) {
    		writer.write(Integer.toString(rand.nextInt(100)));
    		writer.write(",");
    		writer.write(Integer.toString(rand.nextInt(200)));
    		writer.write("\n");
    	}
    	writer.close();

        JolSystem system = Runtime.create(true, 5000);
        system.install("test", ClassLoader.getSystemResource("jol/test/largedata.olg"));
        system.evaluate();
        system.evaluate();
        system.shutdown();


    	for (String[] test : Tester.TESTS) {
    		URL file = ClassLoader.getSystemResource(test[0]);
    		Tester tester = new Tester(file, new TableName(test[1]));
        	tester.setup();
        	tester.test();
        	tester.shutdown();
    	}
    }

}
