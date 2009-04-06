package org.apache.hadoop.mapred.declarative.table;

import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskReport;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.util.FileInput;
import org.apache.hadoop.mapred.declarative.util.TaskState;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.BadKeyException;
import jol.types.table.ConcurrentHashIndex;
import jol.types.table.ConcurrentTable;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.TableName;

public class TaskTable extends ConcurrentTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "task");

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
		Index index = new ConcurrentHashIndex(context, this, typeKey, Index.Type.SECONDARY);
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
	
	public TaskReport[] mapReports(JobID jobid) {
		try {
			Key key = new Key(Field.JOBID.ordinal(), Field.TYPE.ordinal());
			TupleSet tuples = secondary().get(key).lookupByKey(jobid, TaskType.MAP);
			return report(tuples);
		} catch (Throwable e) {
			e.printStackTrace();
		}
		return null;
	}

	public TaskReport[] reduceReports(JobID jobid) {
		try {
			Key key = new Key(Field.JOBID.ordinal(), Field.TYPE.ordinal());
			TupleSet tuples = secondary().get(key).lookupByKey(jobid, TaskType.REDUCE);
			return report(tuples);
		} catch (Throwable e) {
			e.printStackTrace();
		}
		return null;
	}

	private TaskReport[] report(TupleSet tuples) {
		List<TaskReport> reports = new ArrayList<TaskReport>();
		for (Tuple tuple : tuples) {
			TaskID    taskid = (TaskID) tuple.value(Field.TASKID.ordinal());
			TaskState status = (TaskState) tuple.value(Field.STATUS.ordinal());

			reports.add(
					new TaskReport(
						taskid, status.progress(), status.state().name(),
						new String[0],  status.start(), status.finish(), null));
		}
		return reports.toArray(new TaskReport[reports.size()]);
	}
}
