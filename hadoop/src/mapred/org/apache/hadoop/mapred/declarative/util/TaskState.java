package org.apache.hadoop.mapred.declarative.util;

import java.util.HashMap;
import java.util.LinkedHashMap;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.TaskAttemptID;
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
			if (state == Constants.TaskState.KILLED) {
				state = Constants.TaskState.FAILED;
			}
			
			this.progress = state == Constants.TaskState.SUCCEEDED ? 1f : progress;
			this.state    = progress >= 1f ? Constants.TaskState.SUCCEEDED : state;
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
	
	private HashMap<TaskAttemptID, AttemptState> attempts;
	
	public TaskState(JobID jobid, TaskID taskid) {
		this.jobid    = jobid;
		this.taskid   = taskid;
		this.attempts = new LinkedHashMap<TaskAttemptID, AttemptState>();
	}
	
	private TaskState(JobID jobid, TaskID taskid, HashMap<TaskAttemptID, AttemptState> attempts) {
		this(jobid, taskid);
		this.jobid = jobid;
		this.taskid = taskid;
		if (attempts != null) this.attempts.putAll(attempts);
	}
	
	private TaskState(TaskState copy) {
		this(copy.jobid, copy.taskid);
		this.attempts = (HashMap<TaskAttemptID, AttemptState>) copy.attempts.clone();
	}
	
	@Override
	public int hashCode() {
		return (this.jobid.toString() + ":" + this.taskid.toString() + ":" + progress()).hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof TaskState) {
			return compareTo((TaskState)o) == 0;
		}
		return false;
	}
	
	@Override
	public TaskState clone() {
		return new TaskState(jobid, taskid, attempts);
	}
	
	public int compareTo(TaskState o) {
		int comparison = this.jobid.compareTo(o.jobid);
		if (comparison != 0) return comparison;
		comparison = this.taskid.compareTo(o.taskid);
		if (comparison != 0) return comparison;
		comparison = this.state().compareTo(o.state());
		if (comparison != 0) return comparison;
		return Float.compare(this.progress(), o.progress());
	}
	
	public JobID jobid() {
		return this.jobid;
	}
	
	public TaskID taskid() {
		return this.taskid;
	}
	
	public String toString() {
		return "[" + state() + ", " + phase() + ", " + progress() + "]";
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
	public TaskState attempt(TaskAttemptID attemptId,
						     Float progress, 
			                 Constants.TaskState state, 
			                 Constants.TaskPhase phase, 
			                 Long start, Long finish) {
		TaskState copy = new TaskState(this);
		AttemptState attempt = new AttemptState(progress, state, phase, start, finish);
		copy.attempt(attemptId, attempt);
		return copy;
	}
	
	private void attempt(TaskAttemptID id, AttemptState attempt) {
		if (this.attempts.containsKey(id)) {
			if (this.attempts.get(id).compareTo(attempt) < 0) {
				this.attempts.remove(id);
			} else return;
		} 
		this.attempts.put(id, attempt); 
	}
	
	/**
	 * How many attempts have been made on this task.
	 * @return
	 */
	public int attempts() {
		return attempts.size();
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
			else if (best.state == Constants.TaskState.FAILED &&
					state.state != Constants.TaskState.FAILED) {
				best = state;
			}
			else if (best.state == Constants.TaskState.SUCCEEDED) {
				return best;
			}
			else if (state.state == Constants.TaskState.SUCCEEDED ||
			         best.progress < state.progress) {
				best = state;
			}
		}
		return best;
	}
}
