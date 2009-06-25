package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.TaskID;

import jol.core.Runtime;
import jol.types.table.BasicTable;
import jol.types.table.Key;
import jol.types.table.TableName;

public class ReduceScheduleTable extends BasicTable {
	
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "reduceSchedule");
	
	/** The primary key (JobID, TaskID, AttemptID, State). Need state due to overwrite issues. */
	public static final Key PRIMARY_KEY = new Key(1);
	
	/** An enumeration of all fields. */
	public enum Field{LOCATION, TASKID, PARTITION, ADDRESS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,   // Location
		TaskID.class,   // Reduce Task ID
		Integer.class,  // Assigned partition
		String.class    // Socket address
	};
	

	public ReduceScheduleTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}

}
