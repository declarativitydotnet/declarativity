package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.declarative.Constants.JobState;

import jol.types.table.EventTable;
import jol.types.table.TableName;

public class JobCompletionTable extends EventTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "jobCompletion");
	
	/** An enumeration of all fields. */
	public enum Field{LOCATION, JOBID, STATE};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,   // Location
		JobID.class,    // Job id
		JobState.class  // Job state
	};
	

	public JobCompletionTable() {
		super(TABLENAME, SCHEMA);
	}

}
