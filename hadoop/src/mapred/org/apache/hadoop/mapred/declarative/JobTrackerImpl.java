package org.apache.hadoop.mapred.declarative;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import jol.core.JolSystem;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.Server;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.StatusHttpServer;
import org.apache.hadoop.mapred.TaskReport;
import org.apache.hadoop.mapred.declarative.table.*;
import org.apache.hadoop.mapred.declarative.test.JobSimulator;
import org.apache.hadoop.mapred.declarative.test.TaskTrackerCluster;
import org.apache.hadoop.net.DNSToSwitchMapping;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.net.NetworkTopology;
import org.apache.hadoop.net.ScriptBasedMapping;
import org.apache.hadoop.util.ReflectionUtils;
import org.apache.hadoop.util.StringUtils;

public class JobTrackerImpl extends JobTracker {

	  /* Some jobs are stored in a local system directory.  We can delete
	   * the files when we're done with the job. */
	  private static final String SUBDIR = "jobTracker";

	  /* Grab the log. */
	  public static final Log LOG = LogFactory.getLog("org.apache.hadoop.mapred.JobTracker");

	  // system directories are world-wide readable and owner readable
	  private static final FsPermission SYSTEM_DIR_PERMISSION =
	    FsPermission.createImmutable((short) 0733); // rwx-wx-wx
	  private static final int SYSTEM_DIR_CLEANUP_RETRY_PERIOD = 10000;

	  private JobTrackerServer masterInterface;

	  /** The JOL system context. */
	  private JolSystem context;

	  private Executor executor;

	  /** The server interface. */
	  private Server server;

	  private FileSystem fs  = null;
	  private Path systemDir = null;
	  private JobConf conf   = null;

	  /** RPC Server port */
	  private int port;

	  /** RPC Host name */
	  private String localMachine;

	  private DNSToSwitchMapping dnsToSwitchMapping;
	  private NetworkTopology clusterMap = new NetworkTopology();

	public JobTrackerImpl(JolSystem context, JobConf conf) throws IOException, ClassNotFoundException {
		super(getDateFormat().format(new Date()));
		this.conf = conf;
		this.context = context;
		this.executor = Executors.newCachedThreadPool();

	    /* Install the declarative definitions. */
	    try {
			install((jol.core.Runtime)context, conf);
		} catch (UpdateException e) {
			throw new IOException(e);
		} catch (JolRuntimeException e) {
			throw new IOException(e);
		}

		  // Set ports, start RPC servers, etc.
	    InetSocketAddress addr = getAddress(conf);
	    this.localMachine = addr.getHostName();
	    this.port = addr.getPort();
	    int handlerCount = conf.getInt("mapred.job.tracker.handler.count", 10);
	    this.masterInterface = new JobTrackerServer(context, this, conf);
	    this.server = RPC.getServer(this.masterInterface,
	    			                addr.getHostName(), addr.getPort(),
	    			                handlerCount, false, conf);

		this.server.start();

	    // The rpc/web-server ports can be ephemeral ports...
	    // ... ensure we have the correct info
	    this.port = server.getListenerAddress().getPort();
	    this.conf.set("mapred.job.tracker", (this.localMachine + ":" + this.port));
	    LOG.info("JobTracker up at: " + this.port);

	    while (true) {
	    	try {
	    		// if we haven't contacted the namenode go ahead and do it
	    		if (fs == null) {
	    			try {
	    				fs = FileSystem.get(conf);
	    			} catch (IOException e) {
	    				fs = FileSystem.getLocal(conf);
	    			}
	    		}
	    		this.systemDir = new Path(conf.get("mapred.system.dir", "/tmp/hadoop/mapred/system"));
	    		this.systemDir = fs.makeQualified(this.systemDir);

	    		// TODO Recovery

	    		fs.delete(systemDir, true);
	    		if (FileSystem.mkdirs(fs, systemDir, new FsPermission(SYSTEM_DIR_PERMISSION))) {
	    			break;
	    		}
	    	} catch (IOException e) {
	    		e.printStackTrace();
	    		java.lang.System.exit(0);
	    	}
	    }
	    // Same with 'localDir' except it's always on the local disk.
	    this.conf.deleteLocalFiles(SUBDIR);

	    this.dnsToSwitchMapping = (DNSToSwitchMapping) ReflectionUtils.newInstance(
	            conf.getClass("topology.node.switch.mapping.impl", ScriptBasedMapping.class,
	                DNSToSwitchMapping.class), conf);

	}

	public JobTrackerServer masterInterface() {
		return this.masterInterface;
	}

	private static SimpleDateFormat getDateFormat() {
		return new SimpleDateFormat("yyyyMMddHHmm");
	}

	private void install(jol.core.Runtime context, JobConf conf) throws UpdateException, JolRuntimeException {
		this.context().catalog().register(new JobTable(context));
		this.context().catalog().register(new TaskAttemptTable(context));
		this.context().catalog().register(new TaskCreate(this));
		// this.context().catalog().register(new AssignTracker(this));
		this.context().catalog().register(new TaskReportTable(context));
		this.context().catalog().register(new TaskTable(context));
		this.context().catalog().register(new TaskTrackerActionTable(context));
		this.context().catalog().register(new TaskTrackerErrorTable(context));
		this.context().catalog().register(new TaskTrackerTable(context));
		this.context().catalog().register(new NetworkTopologyTable(context, this));

		URL program =
			ClassLoader.getSystemClassLoader().getResource(PROGRAM + ".olg");

		this.context.install("hadoop", program);
		this.context.evaluate();

		URL scheduler =
			ClassLoader.getSystemClassLoader().getResource(SCHEDULER + ".olg");
		this.context.install("hadoop", scheduler);
		this.context.evaluate();

		URL policy =
			ClassLoader.getSystemClassLoader().getResource(POLICY + ".olg");
		this.context.install("hadoop", policy);
		this.context.evaluate();
		this.context.start();
	}

	public JobConf conf() {
		return this.conf;
	}

	public Executor executor() {
		return this.executor;
	}

	@Override
	public JolSystem context() {
		return this.context;
	}

	@Override
	public void offerService() throws IOException, InterruptedException {
		synchronized (this) {
			state = State.RUNNING;
		}
		this.server.join();
	}

	@Override
	public void stopTracker() {
		if (this.server != null) {
			LOG.info("Stopping server");
			this.server.stop();
		}
		if (this.context != null) {
			LOG.info("Stopping JOL context");
			this.context.shutdown();
		}
	}

	@Override
	public int getTrackerPort() {
		return this.port;
	}

	@Override
	public FileSystem fileSystem() {
		return this.fs;
	}

	@Override
	public Tuple newJob(JobID jobid) throws IOException {
	    Path localJobFile = this.conf.getLocalPath(SUBDIR  +"/"+jobid + ".xml");
	    Path localJarFile = this.conf.getLocalPath(SUBDIR +"/"+ jobid + ".jar");
	    Path jobFile      = new Path(systemDir(), jobid + "/job.xml");
	    fs.copyToLocalFile(jobFile, localJobFile);
	    JobConf jobConf = new JobConf(localJobFile);

	    // TODO fix to point at the http server port.
	    String url = "http://" + this.localMachine + ":"  +
	                  this.port + "/jobdetails.jsp?jobid=" + jobid;

	    return JobTable.tuple(jobid, jobFile.toString(), jobConf, url);
	}

	@Override
	public Path systemDir() {
		return this.systemDir;
	}

	@Override
	public TaskReport[] getMapTaskReports(JobID jobId) {
		TaskReportTable table =
			(TaskReportTable) this.context.catalog().table(TaskReportTable.TABLENAME);
		return table.mapReports(jobId);
	}

	@Override
	public TaskReport[] getReduceTaskReports(JobID jobId) {
		TaskReportTable table =
			(TaskReportTable) this.context.catalog().table(TaskReportTable.TABLENAME);
		return table.reduceReports(jobId);
	}

	@Override
	public int getNumResolvedTaskTrackers() {
		Table table = this.context.catalog().table(TaskTrackerTable.TABLENAME);
		return table.cardinality().intValue();
	}

	////////////////////////////////////////////////////////////
	// main()
	////////////////////////////////////////////////////////////

	/**
	 * Start the JobTracker process.  This is used only for debugging.  As a rule,
	 * JobTracker should be run as part of the DFS Namenode process.
	 */
	public static void main(String argv[])
	throws IOException, InterruptedException {
		StringUtils.startupShutdownMessage(JobTracker.class, argv, LOG);
		java.lang.System.err.println("STARTING JOBTRACKER");

		boolean debug   = false;
		int clusterSize = 0;
		for (int i = 0; i < argv.length; i++) {
			if (argv[i].startsWith("-d")) {
				debug = true;
				clusterSize = Integer.parseInt(argv[i+1]);
			}
		}

		try {
			JobTrackerImpl tracker = (JobTrackerImpl) startTracker(new JobConf());
			if (debug) {
				TaskTrackerCluster cluster =
					new TaskTrackerCluster(tracker.masterInterface(), clusterSize);
				JobSimulator simulator = new JobSimulator();
				simulator.start();
				tracker.offerService();
			}
			else {
				tracker.offerService();
			}
		} catch (Throwable e) {
			LOG.fatal(StringUtils.stringifyException(e));
			java.lang.System.exit(-1);
		}
	}


}
