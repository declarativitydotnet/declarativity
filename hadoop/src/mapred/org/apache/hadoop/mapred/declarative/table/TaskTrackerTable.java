package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.declarative.Constants.TrackerState;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.exception.UpdateException;
import jol.types.table.BasicTable;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskTrackerTable extends BasicTable {
	
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "taskTracker");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0,1);
	
	/** An enumeration of all clock table fields. */
	public enum Field{
		JTLOCATION,
		TTLOCATION,
		HOST, 
		HTTP_PORT, 
		STATE,
		FAILURES, 
		MAP_COUNT,
		REDUCE_COUNT,
		MAX_MAP, 
		MAX_REDUCE,
		TIMESTAMP
	};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,           // JT Location
		String.class,           // TT Location
		String.class,           // TT hostname
		Integer.class,          // Http port
		TrackerState.class,     // State
		Integer.class,          // Failures
		Integer.class,          // map tasks
		Integer.class,          // reduce tasks
		Integer.class,          // max map tasks
		Integer.class,          // max reduce tasks
		Long.class              // Timestamp
	};
	

	public TaskTrackerTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		// TODO Auto-generated constructor stub
	}
	
	public static Tuple tuple(TaskTrackerStatus status, boolean init) {
		return new Tuple(null, null,
				         status.getHost(), status.getHttpPort(),
				         init ? TrackerState.INITIAL : TrackerState.RUNNING,
						 status.getFailures(),
						 status.countMapTasks(),
						 status.countReduceTasks(),
						 status.getMaxMapTasks(),
						 status.getMaxReduceTasks());
		
	}

}
