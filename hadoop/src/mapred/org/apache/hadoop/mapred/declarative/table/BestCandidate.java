package org.apache.hadoop.mapred.declarative.table;

import java.util.HashMap;
import java.util.Map;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants;

import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.UpdateException;
import jol.types.table.Function;

public class BestCandidate extends Function {

	private class Candidate {
		public TupleSet tuples;
		public Comparable priority;
		public Candidate(Comparable priority) {
			this.tuples = new BasicTupleSet();
			this.priority = priority;
		}
	}

	/** An enumeration of all fields. */
	public enum Field{JOBID, TASKID, TYPE, TRACKERNAME, PRIORITY};

	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,               // Job identifier
		TaskID.class,             // Task identifier
		Constants.TaskType.class, // Task type
		String.class,             // Trackername
		ValueList.class           // The priority
	};

	public BestCandidate() {
		super("bestCandidate", SCHEMA);
	}

	@Override
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		Map<String, Candidate> bestCandidates = new HashMap<String, Candidate>();
		for (Tuple t : tuples) {
			String trackerName  = (String) t.value(Field.TRACKERNAME.ordinal());
			Comparable priority = (Comparable) t.value(Field.PRIORITY.ordinal());
			if (bestCandidates.containsKey(trackerName)) {
				Candidate c = bestCandidates.get(trackerName);
				if (priority.compareTo(c.priority) < 0) {
					/* Better priority */
					c.priority = priority;
					c.tuples.clear();
					c.tuples.add(t);
				}
				else if (priority.compareTo(c.priority) == 0) {
					c.tuples.add(t);
				}
			}
			else {
				Candidate c = new Candidate(priority);
				c.tuples.add(t);
				bestCandidates.put(trackerName, c);
			}
		}

		TupleSet result = new BasicTupleSet();
		for (Candidate best : bestCandidates.values()) {
			result.addAll(best.tuples);
		}
		return result;
	}


}
