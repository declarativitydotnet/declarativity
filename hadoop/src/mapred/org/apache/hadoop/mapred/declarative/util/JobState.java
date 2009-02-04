package org.apache.hadoop.mapred.declarative.util;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.TupleSet;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.table.TaskTable;

public class JobState implements Comparable<JobState> {
	private JobID jobid;
	
	private Constants.JobState state;

	private Set<TaskState> maps;
	
	private Set<TaskState> reduces;
	
	public JobState(JobID jobid) {
		this.jobid   = jobid;
		this.state   = Constants.JobState.PREP;
		this.maps    = new HashSet<TaskState>();
		this.reduces = new HashSet<TaskState>();
	}
	
	public JobState(JobID jobid, Constants.JobState state) {
		this.jobid = jobid;
		this.state = state;
		this.maps    = new HashSet<TaskState>();
		this.reduces = new HashSet<TaskState>();
	}
	
	public JobState(JobID jobid, Set maps) {
		this(jobid, maps, null);
	}
	
	public JobState(JobID jobid, Set<TaskState> maps, Set<TaskState> reduces) {
		this.jobid   = jobid;
		this.state   = Constants.JobState.RUNNING;
		this.maps    = maps;
		this.reduces = reduces;
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
	
	public String toString() {
		JobStatus status = status();
		return this.jobid.toString() + " - " + 
		       this.state + 
		       " MapProgress[" + status.mapProgress() + "]," +
		       " ReduceProgress[" + status.reduceProgress() + "]";
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
	
	public void task(TaskType type, TaskState state) {
		this.state = Constants.JobState.RUNNING;
		if (type == TaskType.MAP) {
			this.maps.add(state);
		}
		else if (type == TaskType.REDUCE) {
			this.reduces.add(state);
		}
		status();
	}
	
	public Constants.JobState state() {
		return this.state;
	}
	
	public void state(Constants.JobState state) {
		this.state = state;
	}
	
	public JobStatus status() {
		float mapProgress    = 0f;
		float reduceProgress = 0f;
		
		if (this.state == Constants.JobState.RUNNING) {
			for (TaskState map : maps) {
				mapProgress += map.progress();
			}
			mapProgress = mapProgress / (float) this.maps.size();
			if (reduces == null) {
				reduceProgress = 1f;
			}
			else {
				for (TaskState reduce : reduces) {
					reduceProgress += reduce.progress();
				}
				reduceProgress = reduceProgress / (float) this.reduces.size();
			}
		}
		else if (this.state == Constants.JobState.SUCCEEDED) {
			mapProgress = reduceProgress = 1f;
		}
		
		if (mapProgress == 1f && reduceProgress == 1f) {
			this.state = Constants.JobState.SUCCEEDED;
		}
		
		return new JobStatus(this.jobid, mapProgress, reduceProgress, this.state.ordinal()+1);
	}

}
