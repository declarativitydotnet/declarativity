package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.util.FileInput;
import org.apache.hadoop.mapred.declarative.util.TaskState;

import jol.core.Runtime;
import jol.types.basic.TupleSet;
import jol.types.basic.Wrapper;
import jol.types.exception.BadKeyException;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskTable extends ObjectTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "task");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0,1);
	
	/** An enumeration of all fields. */
	public enum Field{JOBID, TASKID, TYPE, PARTITION, FILE, MAP_COUNT, STATUS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,     // Job identifier
		TaskID.class,    // Task identifier
		TaskType.class,  // Task type
		Integer.class,   // Partition number
		FileInput.class, // Input file
		Integer.class,   // Map count
		TaskState.class // Task status
	};
	
	public TaskTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		Key typeKey = new Key(Field.TYPE.ordinal());
		Index index = new HashIndex(context, this, typeKey, Index.Type.SECONDARY);
		secondary().put(typeKey, index);
	}

	public TupleSet mapTasks() {
		try {
			return secondary().get(new Key(Field.TYPE.ordinal())).lookupByKey(TaskType.MAP);
		} catch (BadKeyException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	public TupleSet reduceTasks() {
		try {
			return secondary().get(new Key(Field.TYPE.ordinal())).lookupByKey(TaskType.REDUCE);
		} catch (BadKeyException e) {
			e.printStackTrace();
		}
		return null;
	}
}
