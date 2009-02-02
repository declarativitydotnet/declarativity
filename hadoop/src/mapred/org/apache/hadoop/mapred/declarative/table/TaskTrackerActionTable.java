package org.apache.hadoop.mapred.declarative.table;

import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskTrackerAction;
import org.apache.hadoop.mapred.TaskTrackerAction.ActionType;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TaskTrackerActionTable extends ObjectTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "taskTrackerAction");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key();
	
	/** An enumeration of all fields. */
	public enum Field{TRACKERNAME, TYPE, ACTION};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,   // Tracker name
		ActionType.class,     // Action type
		TaskTrackerAction.class   // Action object
	};
	
	private Key nameKey;

	public TaskTrackerActionTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		this.nameKey = new Key(Field.TRACKERNAME.ordinal());
		Index index = new HashIndex(context, this, nameKey, Index.Type.SECONDARY);
		secondary().put(nameKey, index);
	}
	
	public TaskTrackerAction[] actions(String trackerName, TupleSet tuples) {
		List<TaskTrackerAction> actions = new ArrayList<TaskTrackerAction>();
		try {
			for (Tuple tuple : secondary().get(this.nameKey).lookupByKey(trackerName)) {
				TaskTrackerAction action = 
					(TaskTrackerAction) tuple.value(Field.ACTION.ordinal());
				actions.add(action);
				tuples.add(tuple);
			}
		} catch (BadKeyException e) {
			e.printStackTrace();
		}
		return actions.toArray(new TaskTrackerAction[actions.size()]);
	}

}
