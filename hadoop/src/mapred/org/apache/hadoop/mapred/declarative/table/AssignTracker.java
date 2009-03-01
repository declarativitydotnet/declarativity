package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;

import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.Function;

import java.util.HashSet;
import java.util.Set;
import java.util.TreeSet;
import java.util.HashMap;

public class AssignTracker extends Function {
	/** An enumeration of all fields. */
	public enum Field{TRACKERNAME, SLOTS, TASKS};

	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,   // Tracker name
		Integer.class,  // Slots
		Set.class       // Task candidates
	};

	public AssignTracker() {
		super("assignTracker", SCHEMA);
	}

	@Override
	public TupleSet insert(TupleSet insertions, TupleSet conflicts) throws UpdateException {
		TupleSet result = new BasicTupleSet();
		Set<TaskID> assigned = new HashSet<TaskID>();
		for (Tuple tuple : insertions) {
			String      tracker  = (String) tuple.value(Field.TRACKERNAME.ordinal());
			Integer     slots    = (Integer) tuple.value(Field.SLOTS.ordinal());
			Set<TaskID> tasks    = (Set<TaskID>) tuple.value(Field.TASKS.ordinal());
			
			for (TaskID task : tasks) {
				if (slots <= 0) break;
				if (!assigned.contains(task)) {
					slots--;
					result.add(new Tuple(tracker, slots, task));
					assigned.add(task);
				}
			}
		}
		return result;
	}
}
