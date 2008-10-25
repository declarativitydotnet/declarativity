package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskStatus;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskAttemptTable extends ObjectTable {
	
	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "taskAttempt");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0,1,2);
	
	/** An enumeration of all fields. */
	public enum Field{JOBID, TASKID, ATTEMPTID, PROGRESS, STATE, PHASE, 
		              DIAGNOSTICS, TRACKER, START, FINISH};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,    // Job identifier
		TaskID.class,   // Task identifier
		Integer.class,  // Task attempt count
		Float.class,    // Progress
		Enum.class,     // State
		Enum.class,     // Phase 
		String.class,   // Diagnostic string,
		String.class,   // Tracker name
		Long.class,     // Start time
		Long.class      // Finish time
	};
	
	public TaskAttemptTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	/**
	 * Routine accepts a task status object from some
	 * tracker and makes it into a tuple of type
	 * TaskAttemptTable.
	 * @param status Task status from tracker.
	 * @return TaskAttemptTable tuple
	 */
	public static Tuple tuple(TaskStatus status) {
		TaskAttemptID attemptid = status.getTaskID();
		JobID         jobid     = attemptid.getJobID();
		TaskID        taskid    = attemptid.getTaskID();
		return new Tuple(jobid, taskid, attemptid.getId(), 
				         status.getProgress(), 
				         status.getRunState(), 
				         status.getPhase(),
				         status.getDiagnosticInfo(), 
				         status.getTaskTracker(), 
				         status.getStartTime(), 
				         status.getFinishTime());
	}

}
