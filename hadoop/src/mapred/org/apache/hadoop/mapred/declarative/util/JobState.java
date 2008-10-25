package org.apache.hadoop.mapred.declarative.util;

import java.util.HashSet;
import java.util.Set;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.table.TaskTable;

public class JobState implements Comparable<JobState> {
	private JobID jobid;
	
	private int state;

	private int mapCount;
	
	private Set<TaskState> maps;
	
	private int reduceCount;
	
	private Set<TaskState> reduces;
	
	public JobState(JobID jobid, int mapCount, int reduceCount) {
		this.jobid       = jobid;
		this.state       = JobStatus.PREP;
		this.mapCount    = mapCount;
		this.maps        = new HashSet<TaskState>();
		this.reduceCount = reduceCount;
		this.reduces     = new HashSet<TaskState>();
	}
	
	public JobState(JobID jobid) {
		this(jobid, 0, 0);
	}
	
	public int compareTo(JobState o) {
		return this.jobid.compareTo(o.jobid);
	}
	
	public JobID jobID() {
		return this.jobid;
	}
	
	public void task(TaskType type, TaskState state) {
		if (type == TaskType.MAP) {
			this.maps.add(state);
		}
		else if (type == TaskType.REDUCE) {
			this.reduces.add(state);
		}
	}
	
	public JobStatus status() {
		float mapProgress    = 0f;
		float reduceProgress = 0f;
		
		if (this.state == JobStatus.RUNNING) {
			for (TaskState map : maps) {
				mapProgress += map.progress();
			}
			mapProgress = mapProgress / (float) this.mapCount;
			for (TaskState reduce : reduces) {
				reduceProgress += reduce.progress();
			}
			reduceProgress = reduceProgress / (float) this.reduceCount;
		}
		else if (this.state == JobStatus.SUCCEEDED) {
			mapProgress = reduceProgress = 1f;
		}
		return new JobStatus(this.jobid, mapProgress, reduceProgress, this.state);
	}

}
