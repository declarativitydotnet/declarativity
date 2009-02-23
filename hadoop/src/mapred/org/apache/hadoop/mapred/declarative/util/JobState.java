package org.apache.hadoop.mapred.declarative.util;

import java.io.IOException;
import java.util.HashSet;
import java.util.Set;

import jol.types.basic.TupleSet;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.table.TaskTable;
import java.util.Map;
import java.util.HashMap;
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
		updateStatus();
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
			if (this.maps.containsKey(state.taskid())) {
				this.maps.remove(state.taskid());
			}
			this.maps.put(state.taskid(), state);
		}
		else if (type == TaskType.REDUCE) {
			if (this.reduces.containsKey(state.taskid())) {
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

}
