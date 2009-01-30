package org.apache.hadoop.mapred.declarative.util;

import java.io.IOException;
import java.util.Random;

import jol.types.basic.ComparableSet;
import jol.types.basic.ValueList;
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
	private final static Random rand = new Random();
	
	public static Long priority(Integer category, JobPriority priority, Long timestamp) {
		return category.longValue() * (priority.ordinal() + timestamp);
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
	
	public static ValueList getLocations(JobClient.RawSplit split) {
		String [] locations = new String[2]; // split.getLocations();
		for (int i = 0; i < locations.length; i++) {
			locations[i] = new String("tracker" + (rand.nextInt(5)) + ".localhost");
		}
		return new ValueList(locations);
	}
	
	public static Object random(ComparableSet objects) {
		return objects.toArray()[rand.nextInt(objects.size())];
	}
}
