package org.apache.hadoop.mapred.declarative.util;

import java.util.HashMap;
import java.util.LinkedHashMap;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants;

public class TaskState implements Comparable<TaskState> {
	private static class AttemptState implements Comparable<AttemptState> {
		public Float progress;
		public Constants.TaskState state;
		public Constants.TaskPhase phase;
		public Long start;
		public Long finish;
		public AttemptState(Float progress, 
				            Constants.TaskState state, 
				            Constants.TaskPhase phase, 
				            Long start, Long finish) {
			this.progress = progress;
			this.state    = state;
			this.phase    = phase;
			this.start    = start;
			this.finish   = finish;
		}
		public int compareTo(AttemptState o) {
			int comparison = 0;
			/* First compare the State. */
			comparison = this.state.compareTo(o.state);
			if (comparison != 0) return comparison;
			/* Now compare the Phase. */
			comparison = this.phase.compareTo(o.phase);
			if (comparison != 0) return comparison;
			/* Now compare the Progress. */
			comparison = this.progress.compareTo(o.progress);
			if (comparison != 0) return comparison;
			/* Now compare the Start time. */
			comparison = this.start.compareTo(o.start);
			/* Now compare the Finish time. */
			if (comparison != 0) return comparison;
			comparison = this.finish.compareTo(o.finish);
			
			return comparison;
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
		int comparison = this.jobid.compareTo(o.jobid);
		if (comparison != 0) return comparison;
		comparison = this.taskid.compareTo(o.taskid);
		if (comparison != 0) return comparison;
		if (best() == o.best()) return 0;
		if (best() == null || o.best() == null) return -1;
		return best().compareTo(o.best());
	}
	
	public String toString() {
		return "Task[" + this.taskid + ", " 
		               + state() + ", " + phase() + "]";
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
			            Constants.TaskState state, 
			            Constants.TaskPhase phase, 
			            Long start, Long finish) {
		AttemptState attempt = new AttemptState(progress, state, phase, start, finish);
		if (this.attempts.containsKey(attemptID)) {
			if (this.attempts.get(attemptID).compareTo(attempt) < 0) {
				this.attempts.remove(attemptID);
			}
			else {
				return; // Keep the better one.
			}
		}
		this.attempts.put(attemptID, attempt); 
	}
	
	public float progress() {
		return attempts.size() == 0 ? 0f : best().progress;
	}
	
	public Constants.TaskState state() {
		return attempts.size() == 0 ? 
				Constants.TaskState.UNASSIGNED : best().state;
	}
	
	public Constants.TaskPhase phase() {
		return attempts.size() == 0 ? 
				Constants.TaskPhase.STARTING : best().phase;
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
			else if (state.state == Constants.TaskState.SUCCEEDED ||
			         best.progress < state.progress) {
				best = state;
			}
		}
		return best;
	}
}
