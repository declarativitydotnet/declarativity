package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.declarative.Constants.TaskPhase;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;

import jol.core.Runtime;
import jol.types.table.BasicTable;
import jol.types.table.Key;
import jol.types.table.TableName;

public class MapCompletionTable extends BasicTable {
	
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "mapCompletion");
	
	/** The primary key (JobID, TaskID, AttemptID, State). Need state due to overwrite issues. */
	public static final Key PRIMARY_KEY = new Key(1);
	
	/** An enumeration of all fields. */
	public enum Field{JOBID, ATTEMPTID, STATE, FILELOCATION};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,         // Job id
		TaskAttemptID.class, // Task attempt count
		TaskState.class,     // State
		String.class         // File location
	};
	

	public MapCompletionTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}
}
