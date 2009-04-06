package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskStatus;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.declarative.Constants.TaskPhase;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;
import org.apache.hadoop.net.NetUtils;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.table.ConcurrentTable;
import jol.types.table.Key;
import jol.types.table.TableName;

public class TaskAttemptTable extends ConcurrentTable {
	
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "taskAttempt");
	
	/** The primary key (JobID, TaskID, AttemptID, State). Need state due to overwrite issues. */
	public static final Key PRIMARY_KEY = new Key(2);
	
	/** An enumeration of all fields. */
	public enum Field{JTLOCATION, TTLOCATION, ATTEMPTID, PROGRESS, STATE, PHASE, 
		              FILELOCATION, START, FINISH};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,        // JT JOL Location
		String.class,        // TT JOL Location
		TaskAttemptID.class, // Task attempt count
		Float.class,         // Progress
		TaskState.class,     // State
		TaskPhase.class,     // Phase 
		String.class,        // File location
		Long.class,          // Start time
		Long.class           // Finish time
	};
	
	public TaskAttemptTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
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
		
        String host = tracker.getHost();
        if (NetUtils.getStaticResolution(tracker.getHost()) != null) {
          host = NetUtils.getStaticResolution(tracker.getHost());
        }
		String httpTaskLogLocation = "http://" + host +  ":" + tracker.getHttpPort(); 
		return new Tuple(null, null, attemptid.getId(), 
				         status.getProgress(), 
				         TaskState.valueOf(status.getRunState().name()), 
				         TaskPhase.valueOf(status.getPhase().name()),
				         httpTaskLogLocation,
				         status.getStartTime(), 
				         status.getFinishTime());
	}

}
