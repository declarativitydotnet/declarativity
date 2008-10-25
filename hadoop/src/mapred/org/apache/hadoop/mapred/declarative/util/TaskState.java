package org.apache.hadoop.mapred.declarative.util;

import java.util.HashMap;
import java.util.LinkedHashMap;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskStatus;

public class TaskState implements Comparable<TaskState> {
	private static class AttemptState {
		public Float progress;
		public TaskStatus.State state;
		public TaskStatus.Phase phase;
		public Long start;
		public Long finish;
		public AttemptState(Float progress, TaskStatus.State state, TaskStatus.Phase phase, 
				            Long start, Long finish) {
			this.progress = progress;
			this.state    = state;
			this.phase    = phase;
			this.start    = start;
			this.finish   = finish;
		}
	}
	private JobID jobid;
	
	private TaskID taskid;
	
	private HashMap<Integer, AttemptState> attempts;
	
	public TaskState(JobID jobid, TaskID taskid) {
		this.jobid    = jobid;
		this.taskid   = taskid;
		this.attempts = new LinkedHashMap<Integer, AttemptState>();
	}
	
	@Override
	public int hashCode() {
		return this.taskid.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof TaskState) {
			return compareTo((TaskState) o) == 0;
		}
		return false;
	}
	
	public int compareTo(TaskState o) {
		int jobCompare = this.jobid.compareTo(o.jobid);
		return jobCompare == 0 ? 
				this.taskid.compareTo(o.taskid) : jobCompare;
	}

	/**
	 * Add an attempt state update to this task state.
	 * The best attempt so far will represent the aggregate
	 * state of this task.
	 * @param attemptID The attempt identifier
	 * @param progress The attempt progress
	 * @param state The attempt state
	 * @param phase The attempt phase
	 * @param start The attempt start time
	 * @param finish The attempt finish time
	 */
	public void attempt(Integer attemptID,
						Float progress, 
			            TaskStatus.State state, 
			            TaskStatus.Phase phase, 
			            Long start, Long finish) {
		if (this.attempts.containsKey(attemptID)) {
			this.attempts.remove(attemptID);
		}
		this.attempts.put(attemptID, 
				          new AttemptState(progress, state, phase, 
				        		           start, finish));
	}
	
	public float progress() {
		return attempts.size() == 0 ? 0f : best().progress;
	}
	
	public TaskStatus.State state() {
		return attempts.size() == 0 ? 
				TaskStatus.State.UNASSIGNED : best().state;
	}
	
	public TaskStatus.Phase phase() {
		return attempts.size() == 0 ? 
				TaskStatus.Phase.STARTING : best().phase;
	}
	
	public long start() {
		return attempts.size() == 0 ? 
				0L : best().start;
	}
	
	public long finish() {
		return attempts.size() == 0 ? 
				0L : best().finish;
	}
	
	private AttemptState best() {
		AttemptState best = null;
		for (AttemptState state : this.attempts.values()) {
			if (best == null) {
				best = state;
			}
			else if (state.state == TaskStatus.State.SUCCEEDED ||
			         best.progress < state.progress) {
				best = state;
			}
		}
		return best;
	}
}
