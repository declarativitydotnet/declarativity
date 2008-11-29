package org.apache.hadoop.mapred.declarative.test;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.mapred.InterTrackerProtocol;

public class TaskTrackerCluster {
	
	private InterTrackerProtocol master;
	
	private Executor executor;
	
	private Set<TaskTracker> trackers;
	
	public TaskTrackerCluster(InterTrackerProtocol master, int size) {
		this.master   = master;
		this.executor = Executors.newCachedThreadPool();
		this.trackers = new HashSet<TaskTracker>();
		for (int i = 0; i < size; i++) {
			this.trackers.add(create("tracker" + i));
		}
	}
	
	private TaskTracker create(String name) {
		TaskTracker tracker = new TaskTracker(name, this.executor, this.master, 5000L);
		tracker.start();
		return tracker;
	}
}
