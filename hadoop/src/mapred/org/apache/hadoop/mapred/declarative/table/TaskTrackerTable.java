package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;

import jol.core.Runtime;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskTrackerTable extends ObjectTable {
	
	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "taskTracker");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all clock table fields. */
	public enum Field{
		TRACKERNAME, 
		HOST, 
		HTTP_PORT, 
		TIMESTAMP,
		FAILURES, 
		MAP_COUNT,
		REDUCE_COUNT,
		MAX_MAP, 
		MAX_REDUCE
	};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,   // Tracker name
		String.class,   // Tracker host
		Integer.class,  // Http port
		Long.class,     // Timestamp
		Integer.class,  // Failures
		Integer.class,  // map tasks
		Integer.class,  // reduce tasks
		Integer.class,  // max map tasks
		Integer.class   // max reduce tasks
	};
	

	public TaskTrackerTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		// TODO Auto-generated constructor stub
	}

}
