package org.apache.hadoop.mapred.declarative.test;

import java.io.IOException;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.mapred.InterTrackerProtocol;
import org.apache.hadoop.mapred.JobConf;

public class TaskTrackerCluster {
	
	private Executor executor;
	
	private Set<TaskTracker> trackers;
	
	public TaskTrackerCluster(JobConf conf, int size) throws IOException {
		this.executor = Executors.newCachedThreadPool();
		this.trackers = new HashSet<TaskTracker>();
		for (int i = 0; i < size; i++) {
			this.trackers.add(create(conf, "tracker" + i));
		}
	}
	
	private TaskTracker create(JobConf conf, String name) throws IOException {
		TaskTracker tracker = new TaskTracker(conf, name, this.executor);
		tracker.start();
		return tracker;
	}
}
