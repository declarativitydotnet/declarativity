package org.apache.hadoop.mapred.declarative;

import jol.types.table.TableName;

public final class Constants {
	
	public final static TableName trackerAction = new TableName("hadoop", "trackerAction");
	
	public static enum TrackerState {
		INITIAL, RUNNING, FAILED
	};
	
	public static enum JobState {
		  PREP, RUNNING, FAILED, SUCCEEDED, KILLED
	};
	
	public static enum TaskType {
		MAP, REDUCE
	};

	// enumeration for reporting current phase of a task.
	public static enum TaskPhase {
		STARTING, MAP, SHUFFLE, SORT, REDUCE
	}

	// what state is the task in?
	public static enum TaskState {
		UNASSIGNED, RUNNING, FAILED, KILLED, COMMIT_PENDING, SUCCEEDED
	}
	  
}
