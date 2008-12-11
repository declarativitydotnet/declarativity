package org.apache.hadoop.mapred.declarative.util;

import java.io.IOException;

import jol.types.basic.Wrapper;

import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobPriority;
import org.apache.hadoop.mapred.KillJobAction;
import org.apache.hadoop.mapred.LaunchTaskAction;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.MapTask;
import org.apache.hadoop.mapred.ReduceTask;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskTrackerAction;

public final class Function {
	
	public static Long priority(Integer category, JobPriority priority, Long timestamp) {
		return category.longValue() + priority.ordinal() + timestamp;
	}

	public static Wrapper<TaskTrackerAction> 
	              launchMap(Wrapper<JobClient.RawSplit> split, String jobFile,
			                TaskID taskId, int attemptId, int partition) {
		if (split.object().getBytes() == null) {
			System.err.println("SPLIT BYTES IS NULL FOR TASK " + taskId);
			System.exit(0);
		}
		try {
			return new Wrapper<TaskTrackerAction>(new LaunchTaskAction(
					new MapTask(jobFile, new TaskAttemptID(taskId, attemptId), partition,
					            split.object().getClassName(), 
					            split.object().getBytes())));
		} catch (IOException e) {
			return null;
		}
	}

	public static Wrapper<TaskTrackerAction> 
	              launchReduce(String jobFile, TaskID taskId, int attemptId,
			                   int partition, int numMaps) {
		return new Wrapper<TaskTrackerAction>(new LaunchTaskAction(
				   new ReduceTask(jobFile, new TaskAttemptID(taskId, attemptId), 
						          partition, numMaps)));
	}
	
	public static Wrapper<TaskTrackerAction> killJob(JobID jobid) {
		return new Wrapper<TaskTrackerAction>(new KillJobAction(jobid));
	}
}
