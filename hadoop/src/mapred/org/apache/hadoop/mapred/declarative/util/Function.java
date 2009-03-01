package org.apache.hadoop.mapred.declarative.util;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Random;
import java.util.Set;

import jol.types.basic.ValueList;

import org.apache.hadoop.io.BytesWritable;
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
		try {
			BytesWritable bw = split.getBytes();
			if (bw == null) {
				throw new RuntimeException("Task " + taskId + " has a null valued split!");
			}
			MapTask task = new MapTask(jobFile, new TaskAttemptID(taskId, attemptId), partition, split.getClassName(), bw);
			return new LaunchTaskAction(task);
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
		String [] locations = split.getLocations();
		return new ValueList<String>(locations);
		/*
		ValueList<String> locations = new ValueList<String>();
		for (int i = 0; i < 3; i++) {
			locations.add("tracker" + rand.nextInt(100) + ".localhost");
		}
		return locations;
		*/
	}
	
	public static Float percentile(Float p, List<Float> values) {
		Collections.sort(values);
		int index = (int) (p * values.size());
		if (index == 0) return null;
		return values.get(index);
	}
	
	public static Object random(Set<Object> objects) {
		return objects.toArray()[rand.nextInt(objects.size())];
	}
}
