package org.apache.hadoop.mapred.declarative.table;

import java.io.IOException;
import java.util.Comparator;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.table.TaskCreate.Field;
import org.apache.hadoop.mapred.declarative.util.TaskState;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.basic.Wrapper;
import jol.types.exception.UpdateException;
import jol.types.table.Function;

import java.util.HashSet;
import java.util.Set;
import java.util.TreeSet;
import java.util.HashMap;

public class AssignTracker extends Function {
	private class TaskAssignment implements Comparable<TaskAssignment> {
		private JobID jobid;
		
		private TaskID taskid;
		
		private Integer priority;
		
		public TaskAssignment(JobID jobid, TaskID taskid, Integer priority) {
			this.jobid  = jobid;
			this.taskid = taskid;
			this.priority = priority;
		}

		public int compareTo(TaskAssignment t) {
			int comparison = this.priority.compareTo(t.priority);
			if (comparison == 0) {
				comparison = this.jobid.compareTo(t.jobid);
				return comparison == 0 ? 
						this.taskid.compareTo(t.taskid) : comparison;
			}
			return comparison;
		}
		
		@Override
		public int hashCode() {
			return (this.jobid.hashCode() + ":" + this.taskid.hashCode()).hashCode();
		}
		
		public boolean equals(Object o) {
			if (o instanceof TaskAssignment) {
				TaskAssignment other = (TaskAssignment) o;
				return this.jobid.equals(other.jobid) &&
						this.taskid.equals(other.taskid);
			}
			return false;
		}
	}
	
	/** An enumeration of all fields. */
	public enum Field{JOBID, TASKID, PRIORITY, TRACKERNAME, SLOTS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,    // Job ID 
		TaskID.class,   // Task ID
		Integer.class,  // Priority
		String.class,   // Tracker name
		Integer.class,  // Tracker slots
	};
	
	private JobTracker jobTracker;

	public AssignTracker(JobTracker jobTracker) {
		super("assignTracker", new TypeList(SCHEMA));
		this.jobTracker = jobTracker;
	}

	@Override
	public TupleSet insert(TupleSet insertions, TupleSet conflicts) throws UpdateException {
		HashMap<String, Integer> slotMap = new HashMap<String, Integer>();
		HashMap<String, TreeSet<TaskAssignment>> assignments = 
			new HashMap<String, TreeSet<TaskAssignment>>();
		for (Tuple tuple : insertions) {
			JobID   jobid    = (JobID) tuple.value(Field.JOBID.ordinal());
			TaskID  taskid   = (TaskID) tuple.value(Field.TASKID.ordinal());
			Integer priority = (Integer) tuple.value(Field.PRIORITY.ordinal());
			String  tracker  = (String) tuple.value(Field.TRACKERNAME.ordinal());
			Integer slots    = (Integer) tuple.value(Field.SLOTS.ordinal());
			
			slotMap.put(tracker, slots);
			if (!assignments.containsKey(tracker)) {
				assignments.put(tracker, new TreeSet<TaskAssignment>());
			}
			assignments.get(tracker).add(new TaskAssignment(jobid, taskid, priority));
		}
		
		TupleSet result = new TupleSet(name());
		Set<TaskAssignment> assigned = new HashSet<TaskAssignment>();
		for (String tracker : slotMap.keySet()) {
			Set<TaskAssignment> taskTrackerAssignments = new HashSet<TaskAssignment>();
			Integer slots = slotMap.get(tracker);
			TreeSet<TaskAssignment> tasks = assignments.get(tracker);
			for (TaskAssignment task : tasks) {
				if (slots == 0) break;
				else if (!assigned.contains(task)) {
					assigned.add(task);
					taskTrackerAssignments.add(task);
					slots--;
				}
			}
			result.addAll(assign(tracker, taskTrackerAssignments, slots));
		}
		return result;
	}
	
	public TupleSet assign(String tracker, Set<TaskAssignment> tasks, Integer slots) {
		TupleSet result = new TupleSet(name());
		for (TaskAssignment task : tasks) {
			result.add(new Tuple(task.jobid, task.taskid, task.priority, tracker, slots));
		}
		return result;
	}

}
