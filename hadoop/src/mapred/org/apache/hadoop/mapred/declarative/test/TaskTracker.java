package org.apache.hadoop.mapred.declarative.test;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.concurrent.Executor;

import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.mapred.Counters;
import org.apache.hadoop.mapred.HeartbeatResponse;
import org.apache.hadoop.mapred.InterTrackerProtocol;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobTracker;
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
import org.apache.hadoop.mapred.JobClient.TaskStatusFilter;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;

class TaskTracker extends Thread {
	
	private static class TaskRunner implements Runnable {
		private TaskTracker tracker;
		private Task task;
		private long start;
		private long finish;
		private long time;
		private float progress;
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
			this.finish = 0L;
			this.time = time;
			this.progress = 0f;
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
				progress = (current - this.start) / (float) this.time;
				progress = Math.min(1f, progress);
			}
			
			return status(progress);
		}
		
		private TaskStatus status(float progress) {
			TaskStatus status = null;
			if (task instanceof MapTask) {
				status = new MapTaskStatus(task.getTaskID(), progress,
						state, "", state.name(), 
						tracker.name(), phase, new Counters()); 
			}
			else if (task instanceof ReduceTask) {
				status = new ReduceTaskStatus(task.getTaskID(), progress,
						state, "", state.name(), 
						tracker.name(), phase, new Counters()); 
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
				try {
					if (task instanceof MapTask) {
						this.phase = TaskStatus.Phase.MAP;
						Thread.sleep(this.time);
						this.state = TaskStatus.State.SUCCEEDED;
					}
					else {
						Long phaseTime = time / 3;
						this.phase = TaskStatus.Phase.SHUFFLE;
						Thread.sleep(phaseTime);
						this.phase = TaskStatus.Phase.SORT;
						Thread.sleep(phaseTime);
						this.phase = TaskStatus.Phase.REDUCE;
						Thread.sleep(phaseTime);
						this.state = TaskStatus.State.COMMIT_PENDING;
					}
				} catch (InterruptedException e) {
					this.state = TaskStatus.State.KILLED;
				}
			}
		}
	}
	
	private static final Long MAX_TASK_TIME = 60000L;
	private static final Long MIN_TASK_TIME = 20000L;
	private static final Random rand = new Random();
	
	private Executor executor;
	
	private InterTrackerProtocol master;
	
	private String name;
	
	private short responseId;
	
	private int maxMapTasks;
	
	private int maxReduceTasks;
	
	private float slotUtility;
	
	private long heartbeatCallDuration;
	
	private Set<TaskRunner> runners;
	
	public TaskTracker(JobConf conf, String name, Executor executor) throws IOException {
		this.name       = name;
		this.executor   = executor;
		this.responseId = 0;
		this.heartbeatCallDuration = 0L;
		this.runners    = new HashSet<TaskRunner>();
		
	    this.maxMapTasks = conf.getInt("mapred.tasktracker.map.tasks.maximum", 2);
        this.maxReduceTasks = conf.getInt("mapred.tasktracker.reduce.tasks.maximum", 2);
        
        this.slotUtility = 0f;
        
		InetSocketAddress jobTrackAddr = JobTracker.getAddress(conf);

	    this.master = (InterTrackerProtocol) 
	      RPC.waitForProxy(InterTrackerProtocol.class,
	                       InterTrackerProtocol.versionID, 
	                       jobTrackAddr, conf);
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
	
	public float slotUtility() {
		return this.slotUtility;
	}
	
	public long heartbeatCallDuration() {
		return this.heartbeatCallDuration;
	}
	
	
	public void run() {
		try {
			TaskTrackerStatus status = status();
			HeartbeatResponse response = 
				this.master.heartbeat(status, true, true, this.responseId);
			do {
				process(response, status);
				if (this.slotUtility == 0f) {
					this.slotUtility = slotRate();
				}
				else {
					this.slotUtility = (this.slotUtility / 3f) + ((2f * slotRate()) / 3f);
				}
				
				int interval = response.getHeartbeatInterval();
				sleep(interval);
				boolean acceptNewTasks = this.runners.size() < 
				                         this.maxMapTasks + this.maxReduceTasks;
				long timestamp = System.currentTimeMillis();
				status = status();
				
				
			    response = this.master.heartbeat(status, false, acceptNewTasks, this.responseId);
			    long duration = System.currentTimeMillis() - timestamp;
			    if (this.heartbeatCallDuration == 0L) {
			    	this.heartbeatCallDuration = duration;
			    }
			    else {
			    	this.heartbeatCallDuration = (this.heartbeatCallDuration / 3) + ((2 * duration) / 3);
			    }
			} while (isInterrupted() == false);
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}
	
	private float slotRate() {
		TaskTrackerStatus status = status();
		return (float) (status.countMapTasks() + status.countReduceTasks()) /
			   (float) (status.getMaxMapTasks() + status.getMaxReduceTasks());
	}
	
	private void process(HeartbeatResponse response, TaskTrackerStatus status) {
		this.responseId = response.getResponseId();
		if (response.getActions() == null) return;
		
		int mapSlots = status.getMaxMapTasks() - status.countMapTasks();
		int reduceSlots = status.getMaxReduceTasks() - status.countReduceTasks();
		
		for (TaskTrackerAction action : response.getActions()) {
			if (action instanceof LaunchTaskAction) {
				LaunchTaskAction launch = (LaunchTaskAction) action;
				if (launch.getTask().isMapTask()) mapSlots--;
				else reduceSlots--;
				launch(launch);
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
		long time = task.isMapTask() ? MIN_TASK_TIME : 2 * MIN_TASK_TIME; //  + Math.abs(rand.nextLong() % MAX_TASK_TIME);
		TaskStatus.State state = TaskStatus.State.UNASSIGNED;
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
					status.getRunState() == TaskStatus.State.FAILED ||
					status.getRunState() == TaskStatus.State.KILLED) {
				done.add(runner);
			}
		}
		this.runners.removeAll(done);
		
		return new TaskTrackerStatus(name(), name() + ".localhost", 
                                     0, reports,  0, 
                                     maxMapTasks, maxReduceTasks);
	}

}
