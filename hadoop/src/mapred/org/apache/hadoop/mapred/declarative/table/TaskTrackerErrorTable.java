package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobTracker;

import jol.core.Runtime;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskTrackerErrorTable extends ObjectTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "taskTrackerError");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key();
	
	/** An enumeration of all fields. */
	public enum Field{LOCATION, TRACKERNAME, CLASS, MESSAGE};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,  // JOL Location
		String.class,  // Tracker name
		String.class,  // Error type
		String.class   // Error message
	};
	

	public TaskTrackerErrorTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}

}
