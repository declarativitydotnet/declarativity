package org.apache.hadoop.mapred.declarative.table;

import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskReport;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;

import jol.core.Runtime;
import jol.types.basic.ConcurrentTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.table.BasicTable;
import jol.types.table.ConcurrentHashIndex;
import jol.types.table.ConcurrentTable;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskReportTable extends ConcurrentTable {

	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "task");

	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0,1);

	/** An enumeration of all fields. */
	public enum Field{JOBID, TASKID, TYPE, PROGRESS, STATE, DIAGNOSTICS, START, FINISH};

	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,     // Job identifier
		TaskID.class,    // Task identifier
		TaskType.class,  // Type
		Float.class,     // Progress
		String.class,    // State
		ValueList.class, // Diagnostics
		Long.class,      // Start time
		Long.class       // Finish time
	};

	public TaskReportTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		Key key = new Key(Field.JOBID.ordinal(), Field.TYPE.ordinal());
		Index index  = new ConcurrentHashIndex(context, this, key, Index.Type.SECONDARY);
		secondary().put(key, index);
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
			TaskID            taskid      = (TaskID) tuple.value(Field.TASKID.ordinal());
			Float             progress    = (Float)  tuple.value(Field.PROGRESS.ordinal());
			String            state       = (String) tuple.value(Field.STATE.ordinal());
			Long              startTime   = (Long)   tuple.value(Field.START.ordinal());
			Long              finishTime  = (Long)   tuple.value(Field.FINISH.ordinal());
			ValueList<String> diagnostics = (ValueList<String>)
										    tuple.value(Field.DIAGNOSTICS.ordinal());

			reports.add(new TaskReport(
						taskid, progress, state,
						diagnostics.toArray(new String[diagnostics.size()]),
						startTime, finishTime, null));
		}
		return reports.toArray(new TaskReport[reports.size()]);
	}
}
