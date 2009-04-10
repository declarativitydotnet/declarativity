package org.apache.hadoop.mapred.declarative.table;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.KillJobAction;
import org.apache.hadoop.mapred.KillTaskAction;
import org.apache.hadoop.mapred.LaunchTaskAction;
import org.apache.hadoop.mapred.MapTask;
import org.apache.hadoop.mapred.ReduceTask;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskTrackerAction;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.TaskTrackerAction.ActionType;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;
import jol.types.table.BasicTable;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskTrackerActionTable extends BasicTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "taskTrackerAction");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key();
	
	/** An enumeration of all fields. */
	public enum Field{LOCATION, TRACKERNAME, TYPE, ACTION};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,             // JOL Location
		String.class,             // TT Name
		ActionType.class,         // Action type
		TaskTrackerAction.class   // Action object
	};
	
	private Key nameKey;

	public TaskTrackerActionTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		this.nameKey = new Key(Field.TRACKERNAME.ordinal());
		Index index = new HashIndex(context, this, nameKey, Index.Type.SECONDARY);
		secondary().put(nameKey, index);
	}
	
	public synchronized TaskTrackerAction[] actions(TaskTrackerStatus status, TupleSet tuples) {
		List<TaskTrackerAction> actions = new ArrayList<TaskTrackerAction>();
		int maps = status.getMaxMapTasks() - status.countMapTasks();
		int reduces = status.getMaxReduceTasks() - status.countReduceTasks();
		try {
			Set<JobID> killjobs = new HashSet<JobID>(); 
			Set<TaskAttemptID> killtasks = new HashSet<TaskAttemptID>(); 
			for (Tuple tuple : secondary().get(this.nameKey).lookupByKey(status.getTrackerName())) {
				TaskTrackerAction action = 
					(TaskTrackerAction) tuple.value(Field.ACTION.ordinal());
				if (action instanceof KillJobAction) {
					killjobs.add(((KillJobAction)action).getJobID());
				}
				else if (action instanceof KillTaskAction) {
					killtasks.add(((KillTaskAction)action).getTaskID());
				}
			}
			
			for (Tuple tuple : secondary().get(this.nameKey).lookupByKey(status.getTrackerName())) {
				TaskTrackerAction action =  (TaskTrackerAction) tuple.value(Field.ACTION.ordinal());
				if (action instanceof LaunchTaskAction) {
					Task t = ((LaunchTaskAction)action).getTask();
					if (killjobs.contains(t.getJobID()) || killtasks.contains(t.getTaskID())) {
						continue;
					}
					else if (t instanceof MapTask) {
						if (maps > 0) maps--;
						else continue;
					}
					else if (t instanceof ReduceTask) {
						if (reduces > 0) reduces--;
						else continue;
					}
				}
				actions.add(action);
				tuples.add(tuple);
			}
		} catch (BadKeyException e) {
			e.printStackTrace();
		}
		return actions.toArray(new TaskTrackerAction[actions.size()]);
	}

}
