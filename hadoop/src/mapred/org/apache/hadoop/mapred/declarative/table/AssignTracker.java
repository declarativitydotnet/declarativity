package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;

import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.UpdateException;
import jol.types.table.Function;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.HashMap;

public class AssignTracker extends Function {
	
	private class Assignment implements Comparable<Assignment> {
		public String trackerName;
		public Integer slots;
		public ValueList priority;
		public TaskType type;
		public Set<TaskID> tasks;
		public Assignment(String trackerName, Integer slots, ValueList priority, TaskType type, Set<TaskID> tasks) {
			this.trackerName = trackerName;
			this.slots = slots;
			this.priority = priority;
			this.type = type;
			this.tasks = tasks;
		}
		@Override
		public int compareTo(Assignment o) {
			int comp = this.priority.compareTo(o.priority);
			if (comp != 0) return comp;
			comp = this.slots.compareTo(o.slots);
			if (comp != 0) return comp * -1; // More slots == less loaded machine
			return equals(o) ? 0 : 1;
		}
		
	}
	
	/** An enumeration of all fields. */
	public enum Field{TRACKERNAME, SLOTS, PRIORITY, TYPE, TASKS};

	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,    // Tracker name
		Integer.class,   // Slots
		ValueList.class, // Priority
		TaskType.class, // Task types
		Set.class        // Task candidates
	};

	public AssignTracker() {
		super("assignTracker", SCHEMA);
	}

	@Override
	public TupleSet insert(TupleSet insertions, TupleSet conflicts) throws UpdateException {
		TupleSet result = new BasicTupleSet();
		TreeSet<Assignment> assignments = new TreeSet<Assignment>();
		for (Tuple tuple : insertions) {
			String      tracker  = (String) tuple.value(Field.TRACKERNAME.ordinal());
			Integer     slots    = (Integer) tuple.value(Field.SLOTS.ordinal());
			ValueList   priority = (ValueList) tuple.value(Field.PRIORITY.ordinal());
			TaskType    type     = (TaskType) tuple.value(Field.TYPE.ordinal());
			Set<TaskID> tasks    = (Set<TaskID>) tuple.value(Field.TASKS.ordinal());
			assignments.add(new Assignment(tracker, slots, priority, type, tasks));
		}
		
		Map<String, Integer> trackerMapSlotCount = new HashMap<String, Integer>();
		Map<String, Integer> trackerReduceSlotCount = new HashMap<String, Integer>();
		Set<TaskID> assigned = new HashSet<TaskID>();
		Iterator<Assignment> iter = assignments.iterator();
		while (iter.hasNext()) {
			Assignment assignment = iter.next();
			String      tracker  = assignment.trackerName;
			Integer     slots    = assignment.slots;
			Set<TaskID> tasks    = assignment.tasks;
			
			
			if (assignment.type == TaskType.MAP && trackerMapSlotCount.containsKey(tracker)) {
				slots = trackerMapSlotCount.get(tracker);
			} else if (assignment.type == TaskType.REDUCE && trackerReduceSlotCount.containsKey(tracker)) {
				slots = trackerReduceSlotCount.get(tracker);
			}
			
			for (TaskID task : tasks) {
				if (slots <= 0) break;
				if (!assigned.contains(task)) {
					slots--;
					result.add(new Tuple(tracker, slots, assignment.priority, assignment.type, task));
					assigned.add(task);
				}
			}
			if (assignment.type == TaskType.MAP) trackerMapSlotCount.put(tracker, slots);
			if (assignment.type == TaskType.REDUCE) trackerReduceSlotCount.put(tracker, slots);
		}
		return result;
	}
}
