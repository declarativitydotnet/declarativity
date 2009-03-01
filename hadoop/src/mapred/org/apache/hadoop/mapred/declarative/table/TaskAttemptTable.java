package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskStatus;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.declarative.Constants.TaskPhase;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;
import org.apache.hadoop.net.NetUtils;

import jol.core.Runtime;
import jol.types.basic.ConcurrentTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.BasicTable;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskAttemptTable extends BasicTable {
	
	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "taskAttempt");
	
	/** The primary key (JobID, TaskID, AttemptID, State). Need state due to overwrite issues. */
	public static final Key PRIMARY_KEY = new Key(0,1,2,4);
	
	/** An enumeration of all fields. */
	public enum Field{JOBID, TASKID, ATTEMPTID, PROGRESS, STATE, PHASE, 
		              DIAGNOSTICS, TRACKER, TASKLOCATION, START, FINISH, TIMESTAMP, DIRTYBIT};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,    // Job identifier
		TaskID.class,   // Task identifier
		Integer.class,  // Task attempt count
		Float.class,    // Progress
		TaskState.class, // State
		TaskPhase.class, // Phase 
		String.class,   // Diagnostic string,
		String.class,   // Tracker name
		String.class,   // Task file location
		Long.class,     // Start time
		Long.class,     // Finish time
		Long.class,    // Timestamp (when we got our last update)
		Boolean.class  // Dirty bit
	};
	
	public TaskAttemptTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA, new ConcurrentTupleSet(TABLENAME));
	}
	
	/**
	 * Routine accepts a task status object from some
	 * tracker and makes it into a tuple of type
	 * TaskAttemptTable.
	 * @param status Task status from tracker.
	 * @return TaskAttemptTable tuple
	 */
	public static Tuple tuple(TaskTrackerStatus tracker, TaskStatus status) {
		TaskAttemptID attemptid = status.getTaskID();
		JobID         jobid     = attemptid.getJobID();
		TaskID        taskid    = attemptid.getTaskID();
		
        String host = tracker.getHost();
        if (NetUtils.getStaticResolution(tracker.getHost()) != null) {
          host = NetUtils.getStaticResolution(tracker.getHost());
        }
		String httpTaskLogLocation = "http://" + host +  ":" + tracker.getHttpPort(); 
		return new Tuple(jobid, taskid, attemptid.getId(), 
				         status.getProgress(), 
				         TaskState.valueOf(status.getRunState().name()), 
				         TaskPhase.valueOf(status.getPhase().name()),
				         status.getDiagnosticInfo(), 
				         status.getTaskTracker(), 
				         httpTaskLogLocation,
				         status.getStartTime(), 
				         status.getFinishTime(),
				         System.currentTimeMillis(),
				         true);
	}

}
