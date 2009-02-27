package org.apache.hadoop.mapred.declarative.util;

import java.io.IOException;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.Constants;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class JobState implements Comparable<JobState> {
	private JobID jobid;
	
	private Constants.JobState state;

	private int mapCount;
	private Map<TaskID, TaskState> maps;
	
	private int reduceCount;
	private Map<TaskID, TaskState> reduces;
	
	private JobStatus status;
	
	public JobState(JobID jobid, JobConf conf) {
		this.jobid   = jobid;
		this.state   = Constants.JobState.PREP;
		this.maps    = new ConcurrentHashMap<TaskID, TaskState>();
		this.reduces = new ConcurrentHashMap<TaskID, TaskState>();
		this.mapCount = conf.getNumMapTasks();
		this.reduceCount = conf.getNumReduceTasks();
		System.err.println("JOB " + jobid + " maps = " + this.mapCount + ", reduces = " + conf.getNumReduceTasks());
		this.updateStatus();
	}
	
	private JobState(JobID jobid, int maps, int reduces) {
		this.jobid = jobid;
		this.state   = Constants.JobState.PREP;
		this.maps    = new ConcurrentHashMap<TaskID, TaskState>();
		this.reduces = new ConcurrentHashMap<TaskID, TaskState>();
		this.mapCount = maps;
		this.reduceCount = reduces;
		this.updateStatus();
	}
	
	@Override
	public int hashCode() {
		return this.jobid.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof JobState) {
			return compareTo((JobState)o) == 0;
		}
		return false;
	}
	
	@Override
	public String toString() {
		return this.state.name();
	}
	
	public int compareTo(JobState o) {
		int comparison = this.jobid.compareTo(o.jobid);
		if (comparison != 0) return comparison;
		JobStatus me = status();
		JobStatus other = o.status();
		if (me.getRunState() != other.getRunState()) 
			return me.getRunState() - other.getRunState();
		comparison = Float.compare(me.mapProgress(), other.mapProgress());
		if (comparison != 0) return comparison;
		return Float.compare(me.reduceProgress(), other.reduceProgress());
	}
	
	public JobID jobID() {
		return this.jobid;
	}
	
	public int mapCount() {
		return this.mapCount;
	}
	
	public int reduceCount() {
		return this.reduceCount;
	}
	
	public boolean task(TaskType type, TaskState state) throws IOException {
		if (this.state == Constants.JobState.PREP) {
			this.state = Constants.JobState.RUNNING;
		}
		
		if (!state.jobid().equals(state.jobid())) {
			throw new IOException("JobState: Task state is not part of this job!");
		}
		
		Constants.JobState beforeState = this.state;
		if (type == TaskType.MAP) {
			if (this.maps.containsKey(state.taskid()) &&
					this.maps.get(state.taskid()).compareTo(state) <= 0) {
				this.maps.remove(state.taskid());
			}
			this.maps.put(state.taskid(), state);
		}
		else if (type == TaskType.REDUCE) {
			if (this.reduces.containsKey(state.taskid()) &&
					this.reduces.get(state.taskid()).compareTo(state) <= 0) {
				this.reduces.remove(state.taskid());
			}
			this.reduces.put(state.taskid(), state);
		}
		updateStatus();
		return state() != beforeState; 
	}
	
	public Constants.JobState state() {
		return this.state;
	}
	
	public boolean killjob() {
		if (state() != Constants.JobState.FAILED) {
			this.state = Constants.JobState.FAILED;
			updateStatus();
			return true;
		}
		return false;
	}
	
	public synchronized JobStatus status() {
		return this.status;
	}
	
	private void updateStatus() {
		float mapProgress    = 0f;
		float reduceProgress = 0f;
		
		if (this.state == Constants.JobState.RUNNING) {
			if (this.mapCount == 0) {
				/* No maps so progress is 1. */
				mapProgress = 1f;
			}
			else {
				for (TaskState map : maps.values()) {
					mapProgress += map.progress();
				}
				mapProgress = mapProgress / (float) this.mapCount;
			}
			
			if (this.reduceCount == 0) {
				/* No reduces so progress is 1. */
				reduceProgress = 1f;
			}
			else {
				for (TaskState reduce : reduces.values()) {
					reduceProgress += reduce.progress();
				}
				reduceProgress = reduceProgress / (float) this.reduceCount;
			}
		}
		else if (this.state == Constants.JobState.SUCCEEDED) {
			mapProgress = reduceProgress = 1f;
		}
		
		if (mapProgress == 1f && reduceProgress == 1f) {
			this.state = Constants.JobState.SUCCEEDED;
		}
		int jobState = JobStatus.PREP;
		switch (this.state) {
		case PREP:      jobState = JobStatus.PREP;     break;
		case RUNNING:   jobState = JobStatus.RUNNING;  break;
		case FAILED:    jobState = JobStatus.FAILED;   break;
		case SUCCEEDED: jobState = JobStatus.SUCCEEDED; break;
		default:        System.err.println("UNKNOWN JOB STATE " + this.state);
						jobState = JobStatus.FAILED;
		}
		this.status = new JobStatus(this.jobid, mapProgress, reduceProgress, jobState);
	}
	
	public static void main(String[] args) throws IOException {
		JobID jobid = new JobID("test", 0);
		TaskID taskid = new TaskID("test", 0, true, 0);
		
		JobState jstate = new JobState(jobid, 1, 1);
		TaskState state = new TaskState(jobid, taskid);
		state.attempt(0, 0.0f, Constants.TaskState.RUNNING, Constants.TaskPhase.MAP, 1L, 0L);
		System.err.println(state);
		jstate.task(TaskType.MAP, state);
		
		state = new TaskState(jobid, taskid);
		state.attempt(0, 1.0f, Constants.TaskState.SUCCEEDED, Constants.TaskPhase.MAP, 1L, 2L);
		System.err.println(state);
		jstate.task(TaskType.MAP, state);
		if (jstate.state() == Constants.JobState.SUCCEEDED) {
			System.err.println("EXPECTED NOT SUCCEEDED 1");
		}
		else {
			System.err.println("COOL 1");
		}
		
		state = new TaskState(jobid, taskid);
		state.attempt(0, 0.0f, Constants.TaskState.RUNNING, Constants.TaskPhase.MAP, 1L, 0L);
		System.err.println(state);
		jstate.task(TaskType.MAP, state);
		if (jstate.state() == Constants.JobState.SUCCEEDED) {
			System.err.println("EXPECTED NOT SUCCEEDED 2");
		}
		else {
			System.err.println("COOL 2");
		}
	}

}
