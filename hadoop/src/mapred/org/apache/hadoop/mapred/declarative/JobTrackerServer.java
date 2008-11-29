package org.apache.hadoop.mapred.declarative;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;

import jol.core.System;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;

import org.apache.hadoop.mapred.ClusterStatus;
import org.apache.hadoop.mapred.Counters;
import org.apache.hadoop.mapred.DisallowedTaskTrackerException;
import org.apache.hadoop.mapred.HeartbeatResponse;
import org.apache.hadoop.mapred.InterTrackerProtocol;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobPriority;
import org.apache.hadoop.mapred.JobProfile;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.JobSubmissionProtocol;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskStatus;
import org.apache.hadoop.mapred.ReinitTrackerAction;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskCompletionEvent;
import org.apache.hadoop.mapred.TaskReport;
import org.apache.hadoop.mapred.TaskTrackerAction;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.JobHistory.JobInfo;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.table.JobTable;
import org.apache.hadoop.mapred.declarative.table.NetworkTopologyTable;
import org.apache.hadoop.mapred.declarative.table.TaskAttemptTable;
import org.apache.hadoop.mapred.declarative.table.TaskTable;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerActionTable;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerErrorTable;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerTable;
import org.apache.hadoop.mapred.declarative.util.JobState;
import org.apache.hadoop.util.HostsFileReader;
import org.apache.hadoop.util.VersionInfo;

public class JobTrackerServer implements JobSubmissionProtocol, InterTrackerProtocol {
	private class TaskAttemptListener extends jol.types.table.Table.Callback {
		private HashMap<JobID, List<TaskCompletionEvent>> completionEvents;

		public TaskAttemptListener(HashMap<JobID, List<TaskCompletionEvent>> events) {
			this.completionEvents = events;
		}
		
		@Override
		public void deletion(TupleSet tuples) {
		}
		@Override
		public void insertion(TupleSet tuples) {
			for (Tuple tuple : tuples) {
				JobID jobid     = (JobID)     tuple.value(TaskAttemptTable.Field.JOBID.ordinal());
				TaskID taskid   = (TaskID)    tuple.value(TaskAttemptTable.Field.TASKID.ordinal());
				Integer attempt = (Integer)   tuple.value(TaskAttemptTable.Field.ATTEMPTID.ordinal());
				TaskState state = (TaskState) tuple.value(TaskAttemptTable.Field.STATE.ordinal());
				String  taskLoc = (String)    tuple.value(TaskAttemptTable.Field.TASKLOCATION.ordinal());
				
				if (state == TaskState.SUCCEEDED || 
						state == TaskState.KILLED || 
						state == TaskState.FAILED) {
					synchronized (JobTrackerServer.this.completionEvents) {
						Table table = JobTrackerServer.this.context.catalog().table(TaskTable.TABLENAME);
						TupleSet lookup;
						try { lookup = table.primary().lookupByKey(jobid, taskid);
						} catch (BadKeyException e) {
							JobTrackerImpl.LOG.error(e.toString());
							continue;
						}
						if (lookup.size() != 1) {
							JobTrackerImpl.LOG.error("Task lookup size != 1");
							continue;
						}

						Tuple taskTuple = lookup.iterator().next();
						Integer partition = (Integer) taskTuple.value(TaskTable.Field.PARTITION.ordinal());
						TaskType type     = (TaskType) taskTuple.value(TaskTable.Field.TYPE.ordinal());
						int eventID = JobTrackerServer.this.completionEvents.get(jobid).size();
						TaskCompletionEvent event = 
							new TaskCompletionEvent(eventID,
									new TaskAttemptID(taskid, attempt),
									partition, type == TaskType.MAP,
									TaskCompletionEvent.Status.valueOf(state.name()),
									taskLoc);
						JobTrackerServer.this.completionEvents.get(jobid).add(event);
					}
				}
			}
		}
	}
	
	private class JobListener extends jol.types.table.Table.Callback {

		@Override
		public void deletion(TupleSet tuples) {
			synchronized (JobTrackerServer.this.completionEvents) {
				for (Tuple tuple : tuples) {
					JobID jobid = (JobID) tuple.value(JobTable.Field.JOBID.ordinal());
					JobState state = (JobState) tuple.value(JobTable.Field.STATUS.ordinal());
					if (state.state() == Constants.JobState.SUCCEEDED ||
					    state.state() == Constants.JobState.FAILED) {
						JobTrackerServer.this.completionEvents.remove(jobid);
					}
				}
			}
		}

		@Override
		public void insertion(TupleSet tuples) {
			synchronized (JobTrackerServer.this.completionEvents) {
				for (Tuple tuple : tuples) {
					JobID jobid = (JobID) tuple.value(JobTable.Field.JOBID.ordinal());
					JobState state = (JobState) tuple.value(JobTable.Field.STATUS.ordinal());
					if (state.state() == Constants.JobState.PREP) {
						JobTrackerServer.this.completionEvents.put(jobid, 
								new ArrayList<TaskCompletionEvent>());
					}
				}
			}
		}
		
	}
	
	private boolean debug = false;
	
	private HashMap<String, HeartbeatResponse> heartbeats;
	
	private int nextJobId = 1;

	private System context;
	
	private JobTracker jobTracker;
	
	private HostsFileReader hostsReader;
	
	private HashMap<JobID, List<TaskCompletionEvent>> completionEvents;

	public JobTrackerServer(System context, JobTracker jobTracker, JobConf conf) throws IOException {
		this.context = context;
		this.jobTracker = jobTracker;
		this.heartbeats = new HashMap<String, HeartbeatResponse>();
		this.completionEvents = new HashMap<JobID, List<TaskCompletionEvent>>();
		context.catalog().table(TaskAttemptTable.TABLENAME).register(new TaskAttemptListener(this.completionEvents));
		context.catalog().table(JobTable.TABLENAME).register(new JobListener());
		
		// Read the hosts/exclude files to restrict access to the jobtracker.
	    this.hostsReader = new HostsFileReader(conf.get("mapred.hosts", ""),
	                                           conf.get("mapred.hosts.exclude", ""));
	}

	public JobID getNewJobId() throws IOException {
	    return new JobID(this.jobTracker.identifier(), this.nextJobId++);
	}


	public long getProtocolVersion(String protocol, 
			long clientVersion) throws IOException {
		if (protocol.equals(InterTrackerProtocol.class.getName())) {
			return InterTrackerProtocol.versionID;
		} else if (protocol.equals(JobSubmissionProtocol.class.getName())){
			return JobSubmissionProtocol.versionID;
		} else {
			throw new IOException("Unknown protocol to job tracker: " + protocol);
		}
	}

	  
	/** Yet another method that does not belong in the job submission protocol. 
	 * Why should clients be burdened with even the mere knowledge of tasks. They
	 * should only know about jobs.
	 */
	public boolean killTask(TaskAttemptID arg0, boolean arg1)
	throws IOException {
		return false;
	}
	
	/** stupid */
	public TaskReport[] getCleanupTaskReports(JobID jobid) throws IOException {
		return null;
	}
	
	/** stupid */
	public String[] getTaskDiagnostics(TaskAttemptID arg0) throws IOException {
		return null;
	}
	
	/** i mean really */
	public TaskReport[] getMapTaskReports(JobID arg0) throws IOException {
		// TODO Auto-generated method stub
		return null;
	}

	/** stupid */
	public TaskReport[] getReduceTaskReports(JobID arg0) throws IOException {
		// TODO Auto-generated method stub
		return null;
	}
	
	/** Why should a client know anything about queues or any other system internals!?!?! */
	public JobStatus[] getJobsFromQueue(String queue) throws IOException {
		return null; 
	}

	/** Why should a client know anything about queues or any other system internals!?!?! */
	public JobInfo getQueueInfo(String queue) throws IOException {
		return null;
	}

	/** Why is this part of the job SUBMISSION interface?!?! */
	public JobInfo[] getQueues() throws IOException {
		return null;
	}
	
	public JobStatus[] getAllJobs() throws IOException {
		Table jobs = context.catalog().table(JobTable.TABLENAME);
		JobStatus[] status = new JobStatus[jobs.cardinality().intValue()];
		int index = 0;
		for (Tuple tuple : jobs.tuples()) {
			JobState state = (JobState) tuple.value(JobTable.Field.STATUS.ordinal());
			status[index++] = state.status();
		}
		return status;
	}

	public ClusterStatus getClusterStatus() throws IOException {
		Table taskTrackerTable = context.catalog().table(TaskTrackerTable.TABLENAME);
		TaskTable taskTable    = (TaskTable) context.catalog().table(TaskTable.TABLENAME);
		int totalMapTaskCapacity    = 0;
		int totalReduceTaskCapacity = 0;
		for (Tuple tracker : taskTrackerTable.tuples()) {
			totalMapTaskCapacity    += (Integer) tracker.value(TaskTrackerTable.Field.MAX_MAP.ordinal());
			totalReduceTaskCapacity += (Integer) tracker.value(TaskTrackerTable.Field.MAX_REDUCE.ordinal());
		}
		
	    return new ClusterStatus(taskTrackerTable.cardinality().intValue(),
		                         taskTable.mapTasks().size(),
		                         taskTable.reduceTasks().size(),
		                         totalMapTaskCapacity,
		                         totalReduceTaskCapacity, 
		                         jobTracker.state());          
	}

	public Counters getJobCounters(JobID arg0) throws IOException {
		return null;
	}

	public JobProfile getJobProfile(JobID jobid) throws IOException {
		Table table = context.catalog().table(JobTable.TABLENAME);
		try {
			TupleSet jobs = table.primary().lookupByKey(jobid);
			if (jobs == null || jobs.size() == 0) {
				throw new IOException("Unkown job identifier! " + jobid);
			}
			return profile(jobs.iterator().next());
		} catch (BadKeyException e) {
			e.printStackTrace();
			return null;
		}
	}

	public JobStatus getJobStatus(JobID jobid) throws IOException {
		Table table = context.catalog().table(JobTable.TABLENAME);
		try {
			TupleSet jobs = table.primary().lookupByKey(jobid);
			if (jobs == null || jobs.size() == 0) {
				throw new IOException("Unkown job identifier! " + jobid);
			}
			JobState state = (JobState) jobs.iterator().next().value(JobTable.Field.STATUS.ordinal());
			return state.status();
		} catch (BadKeyException e) {
			e.printStackTrace();
			return null;
		}
	}

	public JobStatus[] jobsToComplete() throws IOException {
		Table jobs = context.catalog().table(JobTable.TABLENAME);
		List<JobStatus> status = new ArrayList<JobStatus>();
		for (Tuple job : jobs.tuples()) {
			JobState state = (JobState) job.value(JobTable.Field.STATUS.ordinal());
			status.add(state.status());
		}
		return status.toArray(new JobStatus[status.size()]);
	}

	/**
	 * Kill job by scheduling a deletion of the job in the job table.
	 */
	public void killJob(JobID jobid) throws IOException {
		try {
			Table jobTable  = this.context.catalog().table(JobTable.TABLENAME);
			TupleSet lookup = jobTable.primary().lookupByKey(jobid);
			
			if (lookup.size() > 0) {
				/* Schedule the deletion of any jobs returned by index. */
				context.schedule(JobTracker.PROGRAM, JobTable.TABLENAME, null, lookup);
			}
		} catch (BadKeyException e) {
			e.printStackTrace();
		} catch (UpdateException e) {
			throw new IOException(e);
		}
	}

	/**
	 * Submit job by scheduling an insertion of the job in the job table.
	 */
	public JobStatus submitJob(JobID jobid) throws IOException {
		try {
			Tuple job = lookup(jobid);
			if (job == null) {
				/* Create a new job and schedule its insertion into the job table. */
				job = jobTracker.newJob(jobid);
				context.schedule(JobTracker.PROGRAM, JobTable.TABLENAME, 
						         new TupleSet(JobTable.TABLENAME, job), null);
				JobState state = (JobState) job.value(JobTable.Field.STATUS.ordinal());
				return state.status();
			}
			else {
				JobState state = (JobState) job.value(JobTable.Field.STATUS.ordinal());
				return state.status();
			}
		} catch (UpdateException e) {
			throw new IOException(e);
		}
	}

	/** 
	 * Update of the job priority in the job table.
	 */
	public void setJobPriority(JobID jobid, String priority) throws IOException {
		Tuple job = lookup(jobid);
		if (job != null) {
			JobPriority value = JobPriority.valueOf(priority);
			job.value(JobTable.Field.PRIORITY.ordinal(), value);
			try {
				context.schedule(JobTracker.PROGRAM, JobTable.TABLENAME, 
				         new TupleSet(JobTable.TABLENAME, job), null);
			} catch (UpdateException e) {
				throw new IOException(e);
			}
		}
		else {
			throw new IOException("Unknown job identifier! " + jobid);
		}
	}

	/******************************************************************************/
	/** HELPER ROUTINES. JobSubmissionProtocol */
	
	/**
	 * Look up a job via its jobid.
	 * @param jobid The job identifier.
	 * @return The job tuple or null if !exists.
	 */
	private final Tuple lookup(JobID jobid) {
		Table table = context.catalog().table(JobTable.TABLENAME);
		try {
			TupleSet jobs = table.primary().lookupByKey(jobid);
			if (jobs == null || jobs.size() == 0) {
				return null;
			}
			return jobs.iterator().next();
		} catch (BadKeyException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	/**
	 * Construct a job profile object based on the job tuple values.
	 * @param job The tuple containing the job information.
	 * @return A job profile that reflects the job tuple information.
	 */
	private final JobProfile profile(Tuple job) {
		JobID  jobid = (JobID)  job.value(JobTable.Field.JOBID.ordinal());
		String name  = (String) job.value(JobTable.Field.JOBNAME.ordinal());
		String file  = (String) job.value(JobTable.Field.JOBFILE.ordinal());
		String user  = (String) job.value(JobTable.Field.USER.ordinal());
		String url   = (String) job.value(JobTable.Field.URL.ordinal());
		
		return new JobProfile(user, jobid, file, url, name);
	}
	
	/******************************************************************************/
	/** InterTrackerProtocol */
	
	public String getBuildVersion() throws IOException {
	    return VersionInfo.getBuildVersion();
	}
	
	public String getFilesystemName() throws IOException {
		if (this.jobTracker.fileSystem() == null) {
			throw new IllegalStateException("FileSystem object not available yet");
		}
		return this.jobTracker.fileSystem().getUri().toString();
	}

	public String getSystemDir() {
		return this.jobTracker.systemDir().toString();
	}

	public TaskCompletionEvent[] getTaskCompletionEvents(JobID jobid,
			int fromEventId, int maxEvents) throws IOException {
	    TaskCompletionEvent[] events = TaskCompletionEvent.EMPTY_ARRAY;
	    synchronized (completionEvents) {
	    	if (completionEvents.containsKey(jobid) &&
	    			completionEvents.get(jobid).size() > fromEventId) {
	    		int actualMax = Math.min(maxEvents, 
	    				(completionEvents.get(jobid).size() - fromEventId));
	    		events = completionEvents.get(jobid).subList(fromEventId, actualMax + fromEventId).toArray(events);        
	    	}
	    }
	    return events;
	}

	public void reportTaskTrackerError(String taskTracker, 
			                           String errorClass, 
			                           String errorMessage)  throws IOException {
		TupleSet insertions = 
			new TupleSet(TaskTrackerErrorTable.TABLENAME, 
				new Tuple(taskTracker, errorClass, errorMessage));
		try {
			context.schedule(JobTracker.PROGRAM, TaskTrackerErrorTable.TABLENAME, insertions, null);
		} catch (UpdateException e) {
			e.printStackTrace();
			throw new IOException(e.toString());
		}
	}

	public HeartbeatResponse heartbeat(TaskTrackerStatus status,
			                           boolean initialContact, 
			                           boolean acceptNewTasks, 
			                           short responseId) throws IOException { 
		
		// Make sure heartbeat is from a tasktracker allowed by the jobtracker. 
		if (!acceptTaskTracker(status)) {
			throw new DisallowedTaskTrackerException(status);
		}
		
		HeartbeatResponse response    = null;
		TupleSet          actions     = null;
		String            trackerName = status.getTrackerName();
		
		if (debug) {
			java.lang.System.err.println("========================= TASK TRACKER HEARTBEAT " + 
					        responseId + "=========================");
		}
		
		try {
			try {
				// First check if the last heartbeat response got through
				if (!initialContact) {
					// If this isn't the 'initial contact' from the tasktracker,
					// there is something seriously wrong if the JobTracker has
					// no record of the 'previous heartbeat'; if so, ask the 
					// tasktracker to re-initialize itself.
					if (!heartbeats.containsKey(trackerName)) {
						JobTrackerImpl.LOG.info("RESET TRACKER " + trackerName);
						response = 
							new HeartbeatResponse(responseId, 
									new TaskTrackerAction[] {new ReinitTrackerAction()});
						return response;
					} else {
						response = heartbeats.get(trackerName);
						if (response.getResponseId() != responseId) {
							// Means that the previous response was not received.
							JobTrackerImpl.LOG.info("RESEND RESPONSE: " + response.getResponseId());
							return response;  // Send it again.
						}
					}
				}

				// Process this heartbeat 
				short newResponseId = (short)(responseId + 1);
				actions  = new TupleSet(TaskTrackerActionTable.TABLENAME);
				response = new HeartbeatResponse(newResponseId, null);
				if (updateTaskTracker(status, initialContact)) {
					TaskTrackerActionTable table = (TaskTrackerActionTable) 
					     context.catalog().table(TaskTrackerActionTable.TABLENAME);
					/* Grab any actions from the action table. */
					response.setActions(table.actions(trackerName, actions));
				}
				else {
					/* Something went wrong with updating the tracker status. */
					JobTrackerImpl.LOG.info("RESET TRACKER " + trackerName);
					response.setActions(new TaskTrackerAction[] {new ReinitTrackerAction()});
				}
				return response;
			} catch (Exception e) {
				e.printStackTrace();
			}

			JobTrackerImpl.LOG.info("RESET TRACKER " + trackerName);
			response = 
				new HeartbeatResponse(responseId, 
						new TaskTrackerAction[] {new ReinitTrackerAction()});
			return response;
		}
		finally {
			/* Store the response. */
			response(trackerName, response, actions);
			if (debug) {
				/*
				try {
					context.evaluate();
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				*/
				java.lang.System.err.println("========================= END HEARTBEAT " + 
						responseId + "=========================");
			}
		}
	}
	
	/**
	 * Helper routine called to handle heartbeat responses. This routine does
	 * two things. 1. Schedules an insertion of the heartbeat response into 
	 * the {@link HeartbeatTable} 2. Schedules the deletion of the actions (passed in)
	 * that will be handled by the response.
	 * @param trackerName The tracker name.
	 * @param response The response that was sent during the heartbeat.
	 * @param actions The actions contained in the response.
	 * @throws UpdateException
	 */
	private void response(String trackerName, HeartbeatResponse response, TupleSet actions) { 
		try {
			if (response != null) {
				heartbeats.put(trackerName, response);
			}
			
			if (actions != null && actions.size() > 0) {
				context.schedule(JobTracker.PROGRAM, TaskTrackerActionTable.TABLENAME, null, actions);
			}
		} catch (UpdateException e) {
			JobTrackerImpl.LOG.fatal("JOL schedule error!", e);
		}
	}
	
	/**
	 * Update the last recorded status for the given task tracker.
	 * It assumes that the taskTrackers are locked on entry.
	 * @param name The name of the tracker
	 * @param status The new status for the task tracker
	 * @return true success, false failure 
	 */
	private boolean updateTaskTracker(TaskTrackerStatus status, boolean init) { 
		try {
			TaskTrackerTable trackerTable = (TaskTrackerTable)
				this.context.catalog().table(TaskTrackerTable.TABLENAME);
			
			TupleSet trackerLookup = trackerTable.primary().lookupByKey(status.getTrackerName());
			if ( init && trackerLookup.size() > 0 ||
				!init && trackerLookup.size() == 0) {
				return false;
			}
			else if (init) {
				NetworkTopologyTable topologyTable =  (NetworkTopologyTable) 
					this.context.catalog().table(NetworkTopologyTable.TABLENAME);
				topologyTable.resolve(status);
			}
			
			Tuple tracker = 
				new Tuple(status.getTrackerName(),  
						status.getHost(), 
						status.getHttpPort(),  
						java.lang.System.currentTimeMillis(), 
						status.getFailures(),  
						status.countMapTasks(), 
						status.countReduceTasks(), 
						status.getMaxMapTasks(), 
						status.getMaxReduceTasks());

			/* Schedule task tracker status update. */
			this.context.schedule(JobTracker.PROGRAM, 
					TaskTrackerTable.TABLENAME, 
					new TupleSet(TaskTrackerTable.TABLENAME, tracker), 
					null);
			
			/* Schedule updates of all tasks running on this tracker. */
			updateTasks(status);
		} catch (Exception e) {
			JobTrackerImpl.LOG.fatal("TaskTracker update error!", e);
			return false;
		}
		return true;
	}
	
	/**
	 * Schedule the list of task attempt updates.
	 * @param tasks
	 * @throws UpdateException 
	 */
	private void updateTasks(TaskTrackerStatus trackerStatus) 
	throws UpdateException {
		String tracker = trackerStatus.getTrackerName();
		List<TaskStatus> tasks = trackerStatus.getTaskReports();
		TupleSet attempts = new TupleSet(TaskAttemptTable.TABLENAME);
		for (TaskStatus taskStatus : tasks) {
			if (taskStatus.getProgress() >= 1f && taskStatus.getRunState() == TaskStatus.State.RUNNING) {
				taskStatus.setStateString(TaskStatus.State.SUCCEEDED.toString());
			}
			taskStatus.setTaskTracker(tracker);
			attempts.add(TaskAttemptTable.tuple(trackerStatus, taskStatus));
		}
		this.context.schedule(JobTracker.PROGRAM, 
					TaskAttemptTable.TABLENAME, 
					new TupleSet(TaskTrackerTable.TABLENAME, attempts), 
					null);
	}
	
	/*********************** Helper routines copied from Hadoop job tracker *********************************/
	
	/**
	 * Return if the specified tasktracker is in the hosts list, 
	 * if one was configured.  If none was configured, then this 
	 * returns true.
	 */
	private boolean inHostsList(TaskTrackerStatus status) {
		Set<String> hostsList = hostsReader.getHosts();
		return (hostsList.isEmpty() || hostsList.contains(status.getHost()));
	}

	/**
	 * Return if the specified tasktracker is in the exclude list.
	 */
	private boolean inExcludedHostsList(TaskTrackerStatus status) {
		Set<String> excludeList = hostsReader.getExcludedHosts();
		return excludeList.contains(status.getHost());
	}

	/**
	 * Returns true if the tasktracker is in the hosts list and 
	 * not in the exclude list. 
	 */
	private boolean acceptTaskTracker(TaskTrackerStatus status) {
		return (inHostsList(status) && !inExcludedHostsList(status));
	}

}
