package org.apache.hadoop.mapred.declarative.table;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.LaunchTaskAction;
import org.apache.hadoop.mapred.MapTask;
import org.apache.hadoop.mapred.ReduceTask;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskTrackerAction;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.TaskTrackerAction.ActionType;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerActionTable.Field;

import jol.core.Runtime;
import jol.types.basic.ConcurrentTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;
import jol.types.table.BasicTable;
import jol.types.table.ConcurrentHashIndex;
import jol.types.table.ConcurrentTable;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

public class LoadActions extends ConcurrentTable {

	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "loadActions");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all fields. */
	public enum Field{TASKID, TYPE, ACTION};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		TaskID.class,             // TaskID
		Constants.TaskType.class, // Action type
		TaskTrackerAction.class   // Action object
	};
	
	private Key typeKey;

	public LoadActions(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		this.typeKey = new Key(Field.TYPE.ordinal());
		Index index = new ConcurrentHashIndex(context, this, typeKey, Index.Type.SECONDARY);
		secondary().put(typeKey, index);
	}
	
	@Override
	public boolean insert(Tuple t) throws UpdateException {
		return super.insert(t);
	}
	
	@Override
	public boolean delete(Tuple t) throws UpdateException {
		return super.delete(t);
	}
	
	public synchronized List<TaskTrackerAction> actions(Constants.TaskType type, int count, TupleSet tuples) {
		List<TaskTrackerAction> actions = new ArrayList<TaskTrackerAction>();
		try {
			for (Tuple tuple : secondary().get(this.typeKey).lookupByKey(type)) {
				if (count > 0) {
					TaskTrackerAction action = (TaskTrackerAction) tuple.value(Field.ACTION.ordinal());
					actions.add(action);
					tuples.add(tuple);
					count--;
				}
			}
		} catch (BadKeyException e) {
			e.printStackTrace();
		}
		return actions;
	}

}
