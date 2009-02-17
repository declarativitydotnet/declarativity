package org.apache.hadoop.mapred.declarative.test;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.concurrent.Executor;

import org.apache.hadoop.mapred.HeartbeatResponse;
import org.apache.hadoop.mapred.InterTrackerProtocol;
import org.apache.hadoop.mapred.KillJobAction;
import org.apache.hadoop.mapred.LaunchTaskAction;
import org.apache.hadoop.mapred.MapTask;
import org.apache.hadoop.mapred.MapTaskStatus;
import org.apache.hadoop.mapred.ReduceTask;
import org.apache.hadoop.mapred.ReduceTaskStatus;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskStatus;
import org.apache.hadoop.mapred.TaskTrackerAction;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;

class TaskTracker extends Thread {
	
	private static class TaskRunner implements Runnable {
		private TaskTracker tracker;
		private Task task;
		private long start;
		private long time;
		private TaskStatus.State state;
		private TaskStatus.Phase phase;
		
		public TaskRunner(TaskTracker tracker, Task task, long time) {
			this(tracker, task, time, TaskStatus.State.UNASSIGNED);
		}
		
		public TaskRunner(TaskTracker tracker, Task task, 
				          long time, TaskStatus.State state) {
			this.tracker = tracker;
			this.task = task;
			this.start = 0L;
			this.time = time;
			this.state = state;
			this.phase = (task instanceof MapTask) ?
				         TaskStatus.Phase.MAP : TaskStatus.Phase.REDUCE;
		}
		
		public String toString() {
			return "RUNNER TASK " + this.task.getTaskID();
		}
		
		public int hashCode() {
			return this.task.getTaskID().hashCode();
		}
		
		public boolean equals(Object o) {
			if (o instanceof TaskRunner) {
				TaskRunner other = (TaskRunner) o;
				return this.task.getTaskID().equals(other.task.getTaskID());
			}
			return false;
		}
		
		public TaskStatus status() {
			float progress = 0f;
			if (this.state == TaskStatus.State.RUNNING) {
				long current = System.currentTimeMillis();
				if (current - this.start < this.start) {
					progress = 1f;
					this.state = TaskStatus.State.SUCCEEDED;
				}
				else if (this.start > 0L) {
					progress = (current - this.start) / (float) this.time;
				}
			}
			
			return status(progress);
		}
		
		private TaskStatus status(float progress) {
			long finish = this.state == TaskStatus.State.SUCCEEDED ?
					System.currentTimeMillis() : 0L;
					
			TaskStatus status = null;
			if (task instanceof MapTask) {
				status = new MapTaskStatus(task.getTaskID(), progress,
						state, "", state.name(), 
						tracker.name(), phase, null); 
			}
			else if (task instanceof ReduceTask) {
				status = new ReduceTaskStatus(task.getTaskID(), progress,
						state, "", state.name(), 
						tracker.name(), phase, null); 
			}
			else {
				return null;
			}
			status.setStartTime(this.start);
			status.setFinishTime(finish);
			return status;
		}
		
		public void run() {
			if (this.state == TaskStatus.State.UNASSIGNED) {
				this.start = System.currentTimeMillis();
				this.state = TaskStatus.State.RUNNING;
			}
		}
	}
	
	private static final Long MAX_TASK_TIME = 120011L; // Ensure prime
	private static final Long MIN_TASK_TIME = 60000L;
	private static final Random rand = new Random();
	
	private Executor executor;
	
	private InterTrackerProtocol master;
	
	private String name;
	
	private short responseId;
	
	private long interval;
	
	private int maxMapTasks;
	
	private int maxReduceTasks;
	
	private Set<TaskRunner> runners;
	
	public TaskTracker(String name, Executor executor, InterTrackerProtocol master, long interval) {
		this.name       = name;
		this.executor   = executor;
		this.master     = master;
		this.responseId = 0;
		this.interval   = interval;
		this.runners    = new HashSet<TaskRunner>();
		
		this.maxMapTasks    = 1 + rand.nextInt(5);
		this.maxReduceTasks = 1 + rand.nextInt(5);
	}
	
	@Override
	public int hashCode() {
		return this.name.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof TaskTracker) {
			return this.name.equals(((TaskTracker)o).name());
		}
		return false;
	}
	
	public String name() {
		return this.name;
	}
	
	public void run() {
		try {
			HeartbeatResponse response = 
				this.master.heartbeat(status(), true, true, this.responseId);
			do {
				process(response);
				sleep(interval);
				boolean acceptNewTasks = this.runners.size() < 
				                         this.maxMapTasks + this.maxReduceTasks;
			    response = this.master.heartbeat(status(), false, acceptNewTasks, this.responseId);
			} while (isInterrupted() == false);
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
	
	private void process(HeartbeatResponse response) {
		this.responseId = response.getResponseId();
		for (TaskTrackerAction action : response.getActions()) {
			if (action instanceof LaunchTaskAction) {
				launch((LaunchTaskAction) action);
			}
			else if (action instanceof KillJobAction) {
				KillJobAction killAction = (KillJobAction) action;
				Set<TaskRunner> kill = new HashSet<TaskRunner>();
				for (TaskRunner runner : this.runners) {
					if (runner.task.getJobID().equals(killAction.getJobID())) {
						kill.add(runner);
						runner.state = TaskStatus.State.KILLED;
					}
				}
			}
		}
	}
	
	private void launch(LaunchTaskAction action) {
		Task task = action.getTask();
		/*
		if (task.isMapTask()) {
			System.err.println("MAP TASK AT TRACKER " + name() + ".");
		}
		else {
			System.err.println("REDUCE TASK AT TRACKER " + name() + ".");
		}
		*/
		
		long time = MIN_TASK_TIME + (rand.nextLong() % MAX_TASK_TIME);
		TaskStatus.State state =   TaskStatus.State.UNASSIGNED;
		/*(time < (MIN_TASK_TIME + 0.5 * MAX_TASK_TIME)) ?
				TaskStatus.State.FAILED : TaskStatus.State.UNASSIGNED; */
		TaskRunner runner = new TaskRunner(this, task, time, state);
		this.executor.execute(runner);
		this.runners.add(runner);
	}
	
	private TaskTrackerStatus status() {
		List<TaskStatus> reports = new ArrayList<TaskStatus>();
		Set<TaskRunner>  done    = new HashSet<TaskRunner>();
		for (TaskRunner runner : this.runners) {
			TaskStatus status = runner.status();
			reports.add(status);
			if (status.getRunState() == TaskStatus.State.SUCCEEDED ||
					status.getRunState() == TaskStatus.State.FAILED) {
				done.add(runner);
			}
		}
		this.runners.removeAll(done);
		
		return new TaskTrackerStatus(name(), name() + ".localhost", 
                                     0, reports,  0, 
                                     maxMapTasks, maxReduceTasks);
	}

}
