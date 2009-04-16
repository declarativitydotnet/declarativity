package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.TaskTrackerAction;
import org.apache.hadoop.mapred.TaskTrackerAction.ActionType;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.EventTable;
import jol.types.table.TableName;

public class TaskTrackerActionTable extends EventTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "taskTrackerAction");
	
	/** An enumeration of all fields. */
	public enum Field{LOCATION, TRACKERNAME, TYPE, ACTION};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,             // JOL Location
		String.class,             // TT Name
		ActionType.class,         // Action type
		TaskTrackerAction.class   // Action object
	};
	
	public TaskTrackerActionTable(Runtime context) {
		super(TABLENAME, SCHEMA);
	}
}
