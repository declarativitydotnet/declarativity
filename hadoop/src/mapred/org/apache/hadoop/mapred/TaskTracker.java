package org.apache.hadoop.mapred;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.net.InetSocketAddress;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FSError;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.LocalDirAllocator;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.Server;
import org.apache.hadoop.mapred.TaskTrackerImpl.MapOutputServlet;
import org.apache.hadoop.metrics.MetricsContext;
import org.apache.hadoop.metrics.MetricsRecord;
import org.apache.hadoop.metrics.MetricsUtil;
import org.apache.hadoop.metrics.Updater;
import org.apache.hadoop.metrics.jvm.JvmMetrics;
import org.apache.hadoop.net.DNS;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.DiskChecker;
import org.apache.hadoop.util.DiskChecker.DiskErrorException;
import org.apache.log4j.LogManager;



public abstract class TaskTracker implements MRConstants, TaskUmbilicalProtocol {
	static final long WAIT_FOR_DONE = 3 * 1000;

	static enum State {NORMAL, STALE, INTERRUPTED, DENIED}

	public static final Log LOG =
		LogFactory.getLog("org.apache.hadoop.mapred.TaskTracker");

	protected static final String SUBDIR = "taskTracker";
	protected static final String CACHEDIR = "archive";
	protected static final String JOBCACHE = "jobcache";

	protected Server taskReportServer = null;
	protected StatusHttpServer server = null;


	protected InetSocketAddress jobTrackAddr;
	protected InetSocketAddress taskReportAddress;
	protected String taskTrackerName;
	protected String localHostname;
	protected int httpPort;


	protected JobConf originalConf;
	protected JobConf fConf;

	protected int maxCurrentMapTasks;
	protected int maxCurrentReduceTasks;
	
	protected LocalDirAllocator lDirAlloc = 
          new LocalDirAllocator("mapred.local.dir");

	public TaskTracker(JobConf conf, Class outputServlet) throws IOException {
		originalConf = conf;
		maxCurrentMapTasks = conf.getInt(
				"mapred.tasktracker.map.tasks.maximum", 2);
		maxCurrentReduceTasks = conf.getInt(
				"mapred.tasktracker.reduce.tasks.maximum", 2);
		
	    this.jobTrackAddr = JobTracker.getAddress(conf);
	    System.err.println("JOB TRACKER ADDRESS " + this.jobTrackAddr);
	    String infoAddr = 
	      NetUtils.getServerAddress(conf,
	                                "tasktracker.http.bindAddress", 
	                                "tasktracker.http.port",
	                                "mapred.task.tracker.http.address");
	    InetSocketAddress infoSocAddr = NetUtils.createSocketAddr(infoAddr);
	    String httpBindAddress = infoSocAddr.getHostName();
	    int httpPort = infoSocAddr.getPort();
	    this.server = new StatusHttpServer(
	                        "task", httpBindAddress, httpPort, httpPort == 0);
	    int workerThreads = conf.getInt("tasktracker.http.threads", 40);
	    server.setThreads(1, workerThreads);
	    FileSystem local = FileSystem.getLocal(conf);
	    server.setAttribute("task.tracker", this);
	    server.setAttribute("local.file.system", local);
	    server.setAttribute("conf", conf);
	    server.setAttribute("log", LOG);
	    server.setAttribute("localDirAllocator", this.lDirAlloc);
	    server.addServlet("mapOutput", "/mapOutput", outputServlet);
	    server.addServlet("taskLog", "/tasklog", TaskLogServlet.class);
	    server.start();
	    this.httpPort = server.getPort();
	    initialize();

	}

	public abstract Updater getTaskTrackerMetrics();

	public synchronized void initialize() throws IOException {
		// use configured nameserver & interface to get local hostname
		this.fConf = new JobConf(originalConf);
		if (fConf.get("slave.host.name") != null) {
			this.localHostname = fConf.get("slave.host.name");
		}
		if (localHostname == null) {
			this.localHostname =
				DNS.getDefaultHost
				(fConf.get("mapred.tasktracker.dns.interface","default"),
						fConf.get("mapred.tasktracker.dns.nameserver","default"));
		}

		//check local disk
		checkLocalDirs(this.fConf.getLocalDirs());
		fConf.deleteLocalFiles(SUBDIR);

		// bind address
		String address = 
			NetUtils.getServerAddress(fConf,
					"mapred.task.tracker.report.bindAddress", 
					"mapred.task.tracker.report.port", 
			"mapred.task.tracker.report.address");
		InetSocketAddress socAddr = NetUtils.createSocketAddr(address);
		String bindAddress = socAddr.getHostName();
		int tmpPort = socAddr.getPort();
		
		// RPC initialization
		int max = maxCurrentMapTasks > maxCurrentReduceTasks ? 
				maxCurrentMapTasks : maxCurrentReduceTasks;
		this.taskReportServer =
			RPC.getServer(this, bindAddress, tmpPort, max, false, this.fConf);
		this.taskReportServer.start();

		// get the assigned address
		this.taskReportAddress = taskReportServer.getListenerAddress();
		this.fConf.set("mapred.task.tracker.report.address",
				taskReportAddress.getHostName() + ":" + taskReportAddress.getPort());
		LOG.info("TaskTracker up at: " + this.taskReportAddress);

		this.taskTrackerName = "tracker_" + localHostname + ":" + taskReportAddress;
		LOG.info("Starting tracker " + taskTrackerName);

	}
	
	public abstract TaskRunner launch(Task task);
	
	public abstract void kill(TaskRunner runner);

	/** Return the port at which the tasktracker bound to */
	public synchronized InetSocketAddress getTaskTrackerReportAddress() {
		return taskReportAddress;
	}

	public static String getCacheSubdir() {
		return TaskTracker.SUBDIR + Path.SEPARATOR + TaskTracker.CACHEDIR;
	}

	public static String getJobCacheSubdir() {
		return TaskTracker.SUBDIR + Path.SEPARATOR + TaskTracker.JOBCACHE;
	}

	/**
	 * Check if the given local directories
	 * (and parent directories, if necessary) can be created.
	 * @param localDirs where the new TaskTracker should keep its local files.
	 * @throws DiskErrorException if all local directories are not writable
	 */
	protected static void checkLocalDirs(String[] localDirs) 
	throws DiskErrorException {
		boolean writable = false;

		if (localDirs != null) {
			for (int i = 0; i < localDirs.length; i++) {
				try {
					DiskChecker.checkDir(new File(localDirs[i]));
					writable = true;
				} catch(DiskErrorException e) {
					LOG.warn("Task Tracker local " + e.getMessage());
				}
			}
		}

		if (!writable)
			throw new DiskErrorException(
			"all local directories are not writable");
	} 


	public abstract void reportTaskFinished(TaskAttemptID taskid);


	public static class Child {

		public static void main(String[] args) throws Throwable {
			//LogFactory.showTime(false);
			LOG.debug("Child starting");

			JobConf defaultConf = new JobConf();
			String host = args[0];
			int port = Integer.parseInt(args[1]);
			InetSocketAddress address = new InetSocketAddress(host, port);
			TaskAttemptID taskid = TaskAttemptID.forName(args[2]);
			TaskUmbilicalProtocol umbilical =
				(TaskUmbilicalProtocol)RPC.getProxy(TaskUmbilicalProtocol.class,
						TaskUmbilicalProtocol.versionID,
						address,
						defaultConf);

			Task task = umbilical.getTask(taskid);
			JobConf job = new JobConf(task.getJobFile());
			TaskLog.cleanup(job.getInt("mapred.userlog.retain.hours", 24));
			task.setConf(job);

			defaultConf.addResource(new Path(task.getJobFile()));

			// Initiate Java VM metrics
			JvmMetrics.init(task.getPhase().toString(), job.getSessionId());

			try {
				// use job-specified working directory
				FileSystem.get(job).setWorkingDirectory(job.getWorkingDirectory());
				task.run(job, umbilical);             // run the task
			} catch (FSError e) {
				e.printStackTrace();
				LOG.fatal("FSError from child", e);
				umbilical.fsError(taskid, e.getMessage());
			} catch (Throwable throwable) {
				throwable.printStackTrace();
				LOG.warn("Error running child", throwable);
				// Report back any failures, for diagnostic purposes
				ByteArrayOutputStream baos = new ByteArrayOutputStream();
				throwable.printStackTrace(new PrintStream(baos));
				umbilical.reportDiagnosticInfo(taskid, baos.toString());
			} finally {
				RPC.stopProxy(umbilical);
				MetricsContext metricsContext = MetricsUtil.getContext("mapred");
				metricsContext.close();
				// Shutting down log4j of the child-vm... 
				// This assumes that on return from Task.run() 
				// there is no more logging done.
				LogManager.shutdown();
			}
		}
	}
}
