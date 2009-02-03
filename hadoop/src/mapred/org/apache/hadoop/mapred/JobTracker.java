package org.apache.hadoop.mapred;

import java.io.IOException;
import java.net.InetSocketAddress;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.mapred.declarative.JobTrackerImpl;

import jol.core.JolSystem;
import jol.types.basic.Tuple;

/**
 * The job tracker interface.
 *
 * All job trackers implement this interface.
 */
public abstract class JobTracker {
	public static final String PROGRAM   = "jobtracker";
	public static final String SCHEDULER = "scheduler";
	public static final String POLICY    = "basicPolicy";

	public static enum State { INITIALIZING, RUNNING }

	protected State state = State.INITIALIZING;

	protected final String identifier;

	protected final long startTime;

	public JobTracker(String identifier) {
		this.identifier = identifier;
		this.startTime = java.lang.System.currentTimeMillis();
	}

	public static JobTracker startTracker(JobConf conf)
	throws IOException, InterruptedException {
		try {
			JolSystem context = jol.core.Runtime.create();
			return new JobTrackerImpl(context, conf);
		} catch (Throwable e) {
			throw new IOException(e);
		}
	}

	public static InetSocketAddress getAddress(Configuration conf) {
		String address = conf.get("mapred.job.tracker", "localhost:9001");
		return NetUtils.createSocketAddr(address);
	}

	public final String identifier() {
		return this.identifier;
	}

	public final State state() {
		return this.state;
	}

	/**
	 * Starts the job tracker server. This is a blocking call
	 * and will not return until the server is interrupted.
	 * @throws IOException
	 * @throws InterruptedException
	 */
	public abstract void offerService() throws IOException, InterruptedException;

	/**
	 * Halt this job tracker.
	 */
	public abstract void stopTracker();

	/**
	 * Get the DFS
	 * @return The DFS object.
	 */
	public abstract FileSystem fileSystem();

	/**
	 * Get the system directory
	 * @return The DFS system directory.
	 */
	public abstract Path systemDir();

	/**
	 * Get the job tracker server (RPC) port.
	 * @return The job tracker server port.
	 */
	public abstract int getTrackerPort();

	/**
	 * Get the http port.
	 * @return The http port number.
	 */
	public int getInfoPort() { return 0; }

	/**
	 * Get the JOL system context.
	 * @return The JOL system context.
	 */
	public abstract JolSystem context();

	/**
	 * Creates a new job based on the job file.
	 * @param jobid The job identifier (used to locate the job file).
	 * @return A job tuple.
	 * @throws IOException
	 */
	public abstract Tuple newJob(JobID jobid) throws IOException;

	/**
	 * Get a task report for all known map tasks belonging to
	 * a particular job.
	 * @param jobId The job identifier
	 * @return Task reports for all maps that are part of the job.
	 */
	public abstract TaskReport[] getMapTaskReports(JobID jobId);

	/**
	 * Get a task report for all known reduce tasks belonging to
	 * a particular job.
	 * @param jobId The job identifier
	 * @return Task reports for all reduces that are part of the job.
	 */
	public abstract TaskReport[] getReduceTaskReports(JobID jobId);

	/**
	 * Get the number of known task trackers.
	 * @return Number of known task trackers.
	 */
	public abstract int getNumResolvedTaskTrackers();

}
