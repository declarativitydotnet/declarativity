package org.apache.hadoop.mapred.declarative;

public final class Constants {
	
	public static enum JobState {
		  RUNNING, SUCCEEDED, FAILED, PREP
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
		RUNNING, SUCCEEDED, FAILED, UNASSIGNED, KILLED, COMMIT_PENDING
	}
	  
}
