package org.apache.hadoop.mapred.declarative.util;

import java.io.IOException;
import java.util.Random;
import java.util.Set;

import jol.types.basic.ValueList;

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
	
	public static boolean coinFlip() {
		return rand.nextBoolean();
	}
	
	public static Long priority(Integer category, JobPriority priority, Long timestamp) {
		return category.longValue() * (priority.ordinal() + timestamp);
	}

	public static TaskTrackerAction 
	              launchMap(JobClient.RawSplit split, String jobFile,
			                TaskID taskId, int attemptId, int partition) {
		if (split.getBytes() == null) {
			System.err.println("SPLIT BYTES IS NULL FOR TASK " + taskId);
			System.exit(0);
		}
		try {
			return new LaunchTaskAction(
					new MapTask(jobFile, new TaskAttemptID(taskId, attemptId), partition,
					            split.getClassName(), split.getBytes()));
		} catch (IOException e) {
			return null;
		}
	}

	public static TaskTrackerAction 
	              launchReduce(String jobFile, TaskID taskId, int attemptId,
			                   int partition, int numMaps) {
		return new LaunchTaskAction(
				   new ReduceTask(jobFile, new TaskAttemptID(taskId, attemptId), 
						          partition, numMaps));
	}
	
	public static TaskTrackerAction killJob(JobID jobid) {
		return new KillJobAction(jobid);
	}
	
	public static ValueList<String> getLocations(JobClient.RawSplit split) {
		/*
		String [] locations = split.getLocations();
		return new ValueList<String>(locations);
		*/
		ValueList<String> locations = new ValueList<String>();
		for (int i = 0; i < 3; i++) {
			locations.add("tracker" + rand.nextInt(100));
		}
		return locations;
	}
	
	public static Object random(Set<Object> objects) {
		return objects.toArray()[rand.nextInt(objects.size())];
	}
}
