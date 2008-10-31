package jol.net.servlet.util;

import java.util.ArrayList;
import java.util.List;

import jol.core.Runtime;
import jol.core.Runtime.RuntimeCallback;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

/**
 * A set of utility methods that implement a runtime API for Lincoln.
 * 
 *  These APIs are experimental.
 *  
 * @author sears
 *
 */
public class Lincoln {
	
	private Lincoln() {}
	
	/**
	 * One of two types of callbacks that we make use of.  This is just a table 
	 * callback, but it is associated with a particular table, so we can register it 
	 * from inside of evaluateTimestamp().
	 */
	
    public static abstract class Logger extends Callback {
    	private Table t;

    	public Logger(Table t) {
    		this.t = t;
    		if(t == null) 
    			throw new NullPointerException("Can't log against non-existent table");
    	}
    	public Logger(Runtime r, TableName n) {
    		this.t = r.catalog().table(n);
    		if(t == null) {
    			throw new NullPointerException("Table " + n + " not found");
    		}
    	}
    	public Logger(Runtime r, String scope, String name) {
    		this.t = r.catalog().table(new TableName(scope, name));
    		if(t == null) {
    			throw new NullPointerException("Table " + scope
    											 + "::" + name + " not found");
    		}
		}

    	public void start() { t.register(this); }
    	public void stop() { t.unregister(this); }
    }

    /**
     * Atomically evaluate a logical timestamp.  This method waits for any already executing timestamp
     * to complete (we'll call this the 'current' timestamp, then initiates its own evaluation of a
     * logical clock tick. (We'll call this the 'next' timestamp).  The next timestamp will be executed
     * to completion before this function returns.
     * 
     * @param r The runtime object.  It should already be executing in another thread.
     * @param setup An array of runtime callbacks that will be called between the current timestamp
     * 				the next timestamp.
     * 
     * @param loggers An array of table callbacks that will be active during the next timestamp
     * @param teardown An array of runtime callbacks that will be called after the next timestamp has
     * 				   completed, but before another timestamp is initiated.
     * 
     * @throws UpdateException
     * @throws JolRuntimeException
     * 
     * XXX Implement 'disable interrupts' in Runtime.java, and call it from Runtime.evaluateTimestamp().
     */
    public static void evaluateTimestep(Runtime r, final RuntimeCallback[] setup,
			 final Logger[] loggers,
			 final RuntimeCallback[] teardown) 
						throws UpdateException, JolRuntimeException {
		final RuntimeCallback pre = new RuntimeCallback() {
			@Override
			public void call(Runtime r) throws UpdateException, JolRuntimeException {
				for(Logger l : loggers) { l.start(); }
				for(RuntimeCallback c: setup) { c.call(r); }
			}
		};

		final RuntimeCallback post = new RuntimeCallback() {

			@Override
			public void call(Runtime r) throws UpdateException,
			JolRuntimeException {
				for(Logger l : loggers) { l.stop(); }
				for(RuntimeCallback c: teardown) {c.call(r);}
			}
		};
		r.evaluateTimestamp(pre, post);
	}

    public static class DumpTable implements RuntimeCallback {
    	TableName name;
    	ArrayList<Tuple> tupleList;
    	public DumpTable(TableName name) {
    		this.name = name;
    		this.tupleList = new ArrayList<Tuple>();
    	}
		public void call(Runtime r) {
			for(Tuple t : r.catalog().table(name).primary()) {
				tupleList.add(t);
			}
		}
		public ArrayList<Tuple> result() {
			return tupleList;
		}
    }
    public static class InjectTuples implements RuntimeCallback {
    	private String program;
    	private TableName table;
    	private TupleSet insertions;
    	private TupleSet deletions;
    	
    	public InjectTuples(String program, TableName table,
							TupleSet insertions,
							TupleSet deletions) {
			this.program = program;
			this.table = table;
			this.insertions = insertions;
			this.deletions = deletions;
		}

    	public InjectTuples(String program, TableName table) {
    		this.program = program;
    		this.table = table;
    		this.insertions = new TupleSet();
    		this.deletions = new TupleSet();
    	}

    	public TupleSet insertions() { return insertions; }
    	public TupleSet deletions() { return deletions; }
    	
    	@Override
		public void call(Runtime r) throws UpdateException,
				JolRuntimeException {
			r.schedule(program, table, insertions, deletions);
		}
    }
    public static class DeltaLogger extends Logger {
    	public DeltaLogger(Table t) { super(t);	}
    	public DeltaLogger(Runtime r, TableName tn) { super(r, tn);	}
    	public DeltaLogger(Runtime r, String scope, String name) { super(r, scope, name);	}

    	ArrayList<Tuple> deletions = new ArrayList<Tuple>();
    	ArrayList<Tuple> insertions = new ArrayList<Tuple>();
    	
    	@Override
		public void deletion(TupleSet tuples) {
			deletions.addAll(tuples);
		}

		@Override
		public void insertion(TupleSet tuples) {
			insertions.addAll(tuples);
		}
		public List<Tuple> getDeletions() { return deletions; }
		public List<Tuple> getInsertions() { return insertions; }
    }

}
