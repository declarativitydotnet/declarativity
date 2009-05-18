package org.apache.hadoop.mapred.declarative.master;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import jol.core.JolSystem;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.JolRuntimeException;
import jol.types.table.Table;

import org.apache.hadoop.mapred.ClusterStatus;
import org.apache.hadoop.mapred.Counters;
import org.apache.hadoop.mapred.InterTrackerProtocol;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobPriority;
import org.apache.hadoop.mapred.JobProfile;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.JobSubmissionProtocol;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.MRConstants;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskReport;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.table.JobTable;
import org.apache.hadoop.mapred.declarative.table.TaskTable;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerTable;
import org.apache.hadoop.mapred.declarative.util.JobState;
import org.apache.hadoop.util.HostsFileReader;
import org.apache.hadoop.util.VersionInfo;

public class JobTrackerServer implements JobSubmissionProtocol {
	private int nextJobId = 1;

	final private JolSystem context;

	private JobTracker jobTracker;

	private HostsFileReader hostsReader;

	private Table jobTable;

	private Table taskTrackerTable;


	public JobTrackerServer(JolSystem context, JobTracker jobTracker, JobConf conf) throws IOException {
		this.context = context;
		this.jobTracker = jobTracker;

		// Read the hosts/exclude files to restrict access to the jobtracker.
		this.hostsReader = new HostsFileReader(conf.get("mapred.hosts", ""),
				conf.get("mapred.hosts.exclude", ""));

	    this.jobTable = context.catalog().table(JobTable.TABLENAME);
	    this.taskTrackerTable = context.catalog().table(TaskTrackerTable.TABLENAME);
	}
	
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

	public synchronized JobID getNewJobId() throws IOException {
		return new JobID(this.jobTracker.identifier(), this.nextJobId++);
	}


	public synchronized long getProtocolVersion(String protocol, long clientVersion) throws IOException {
		if (protocol.equals(InterTrackerProtocol.class.getName())) {
			return InterTrackerProtocol.versionID;
		} else if (protocol.equals(JobSubmissionProtocol.class.getName())){
			return JobSubmissionProtocol.versionID;
		} else {
			throw new IOException("Unknown protocol to job tracker: " + protocol);
		}
	}


	@Override
	public boolean killTask(TaskAttemptID arg0, boolean arg1)
	throws IOException {
		return false;
	}

	@Override
	public String[] getTaskDiagnostics(TaskAttemptID arg0) throws IOException {
		return null;
	}

	@Override
	public TaskReport[] getMapTaskReports(JobID arg0) throws IOException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public TaskReport[] getReduceTaskReports(JobID arg0) throws IOException {
		// TODO Auto-generated method stub
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
		TaskTable taskTable    = (TaskTable) context.catalog().table(TaskTable.TABLENAME);
		int totalMapTaskCapacity    = 0;
		int totalReduceTaskCapacity = 0;
		for (Tuple tracker : this.taskTrackerTable.tuples()) {
			totalMapTaskCapacity    += (Integer) tracker.value(TaskTrackerTable.Field.MAX_MAP.ordinal());
			totalReduceTaskCapacity += (Integer) tracker.value(TaskTrackerTable.Field.MAX_REDUCE.ordinal());
		}

		return new ClusterStatus(this.taskTrackerTable.cardinality().intValue(),
				taskTable.mapTasks().size(),
				taskTable.reduceTasks().size(),
				totalMapTaskCapacity,
				totalReduceTaskCapacity,
				jobTracker.state());
	}

	public Counters getJobCounters(JobID arg0) throws IOException {
		return new Counters();
	}

	public JobProfile getJobProfile(JobID jobid) throws IOException {
		TupleSet jobs = null;
		try {
			jobs = this.jobTable.primary().lookupByKey(jobid);
		} catch (BadKeyException e) {
			e.printStackTrace();
			return null;
		}

		if (jobs == null || jobs.size() == 0) {
			throw new IOException("Unknown job identifier! " + jobid);
		}
		return profile(jobs.iterator().next());
	}

	public JobStatus getJobStatus(JobID jobid) throws IOException {
		try {
			TupleSet jobs = this.jobTable.primary().lookupByKey(jobid);
			if (jobs == null || jobs.size() == 0) {
				throw new IOException("Unknown job identifier! " + jobid);
			}
			JobState state = (JobState) jobs.iterator().next().value(JobTable.Field.STATUS.ordinal());
			return state.status();
		} catch (BadKeyException e) {
			e.printStackTrace();
			return null;
		}
	}

	public synchronized JobStatus[] jobsToComplete() throws IOException {
		List<JobStatus> status = new ArrayList<JobStatus>();
		for (Tuple job : this.jobTable.tuples()) {
			JobState state = (JobState) job.value(JobTable.Field.STATUS.ordinal());
			status.add(state.status());
		}
		return status.toArray(new JobStatus[status.size()]);
	}

	/**
	 * Kill job by scheduling a deletion of the job in the job table.
	 */
	public synchronized void killJob(JobID jobid) throws IOException {
		try {
			TupleSet lookup = this.jobTable.primary().lookupByKey(jobid);

			if (lookup.size() > 0) {
				/* Schedule the deletion of any jobs returned by index. */
				for (Tuple t : lookup) {
					JobState status = (JobState) t.value(JobTable.Field.STATUS.ordinal());
					if (status != null && status.state() != Constants.JobState.FAILED) {
						status.killjob();
					}
				}
				context.schedule(JobTracker.PROGRAM, JobTable.TABLENAME, lookup, null);
			}
		} catch (Throwable e) {
			e.printStackTrace();
			throw new IOException(e);
		}
	}

	/**
	 * Submit job by scheduling an insertion of the job in the job table.
	 */
	public synchronized JobStatus submitJob(JobID jobid) throws IOException {
		try {
			Tuple job = lookup(jobid);
			if (job == null) {
				TupleSet tasks = jobTracker.createTasks(jobid);

				/* Create a new job and schedule its insertion into the job table. */
				job = jobTracker.newJob(jobid);
				context.schedule(JobTracker.PROGRAM, JobTable.TABLENAME, new BasicTupleSet(job), null);
				context.evaluate();
				
				context.schedule(JobTracker.PROGRAM, TaskTable.TABLENAME, tasks, null);
				context.evaluate();

				JobStatus status = null;
				for (int i = 0; status == null && i < 5; i++) {
					try {
						status = getJobStatus(jobid);
					}
					catch (IOException e) {
						Thread.sleep(MRConstants.HEARTBEAT_INTERVAL_MIN);
						context.evaluate();
					}
				}
				if (status != null) return status;
			}
			JobState state = (JobState) job.value(JobTable.Field.STATUS.ordinal());
			return state.status();
		} catch (Throwable e) {
			e.printStackTrace();
			throw new IOException(e);
		}
	}

	/**
	 * Update of the job priority in the job table.
	 */
	public synchronized void setJobPriority(JobID jobid, String priority) throws IOException {
		Tuple job = lookup(jobid);
		if (job != null) {
			try {
				JobPriority value = JobPriority.valueOf(priority);
				Object [] values = job.toArray();
				values[JobTable.Field.PRIORITY.ordinal()] = value;
				job = new Tuple(values);
				context.schedule(JobTracker.PROGRAM, JobTable.TABLENAME, new BasicTupleSet(job), null);
			} catch (JolRuntimeException e) {
				throw new IOException(e);
			}
		}
		else {
			throw new IOException("Unknown job identifier! " + jobid);
		}
	}

	/*********************** Helper routines copied from Hadoop job tracker *********************************/

	/**
	 * Look up a job via its jobid.
	 * @param jobid The job identifier.
	 * @return The job tuple or null if !exists.
	 */
	private final Tuple lookup(JobID jobid) {
		try {
			TupleSet jobs = this.jobTable.primary().lookupByKey(jobid);
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
