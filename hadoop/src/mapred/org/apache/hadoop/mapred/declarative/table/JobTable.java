package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.declarative.util.Wrapper;

import jol.core.Runtime;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class JobTable extends ObjectTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName("mapred", "job");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all clock table fields. */
	public enum Field{JOBID, NAME, USER, FILE, START_TIME, PRIORITY, URL, RUN_STATE, JOBCONF, MAP_PROGRESS, REDUCE_PROGRESS, CLEAN_PROGRESS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,   // JobID
		String.class,  // Name
		String.class,  // User
		String.class,  // File
		Long.class,    // Start time
		Enum.class,    // Priority
		String.class,  // URL
		Integer.class, // Run state
		Wrapper.class, // JobConf
		Float.class,   // Map progress
		Float.class,   // Reduce progress 
		Float.class    // Cleanup progress
	};
	
	private Runtime context;

	public JobTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		this.context = context;
	}

}
