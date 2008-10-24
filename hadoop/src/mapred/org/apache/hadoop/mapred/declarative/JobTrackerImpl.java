package org.apache.hadoop.mapred.declarative;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;

import jol.core.System;
import jol.types.basic.Tuple;
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

	  /** The JOL system context. */
	  private System context;
	  
	  /** The server interface. */
	  private Server server;

	  private FileSystem fs  = null;
	  private Path systemDir = null;
	  private JobConf conf   = null;
	  
	  /** RPC Server port */
	  private int port;
	  
	  /** RPC Host name */
	  private String localMachine;
	  
	  // Used to provide an HTML view on Job, Task, and TaskTracker structures
	  private StatusHttpServer infoServer;
	  private int infoPort;
	  
	  private DNSToSwitchMapping dnsToSwitchMapping;
	  private NetworkTopology clusterMap = new NetworkTopology();

	public JobTrackerImpl(System context, JobConf conf) throws IOException {
		super(getDateFormat().format(new Date()));
		this.conf = conf;
		
		  // Set ports, start RPC servers, etc.
	    InetSocketAddress addr = getAddress(conf);
	    this.localMachine = addr.getHostName();
	    this.port = addr.getPort();
	    int handlerCount = conf.getInt("mapred.job.tracker.handler.count", 10);
	    this.server = 
	    	RPC.getServer(new JobTrackerServer(context, this, conf), 
	    			      addr.getHostName(), addr.getPort(), 
	    			      handlerCount, false, conf);
	    
	    String infoAddr = 
	    	NetUtils.getServerAddress(conf, "mapred.job.tracker.info.bindAddress",
	    			"mapred.job.tracker.info.port",
	    	"mapred.job.tracker.http.address");
	    InetSocketAddress infoSocAddr = NetUtils.createSocketAddr(infoAddr);
	    String infoBindAddress = infoSocAddr.getHostName();
	    int tmpInfoPort = infoSocAddr.getPort();
	    this.infoServer = new StatusHttpServer("job", infoBindAddress, tmpInfoPort, 
	    		                               tmpInfoPort == 0);
	    this.infoServer.setAttribute("job.tracker", this);
	    
	    
	    // The rpc/web-server ports can be ephemeral ports... 
	    // ... ensure we have the correct info
	    this.port = server.getListenerAddress().getPort();
	    this.conf.set("mapred.job.tracker", (this.localMachine + ":" + this.port));
	    LOG.info("JobTracker up at: " + this.port);
	    this.infoPort = this.infoServer.getPort();
	    this.conf.set("mapred.job.tracker.http.address", 
	    		infoBindAddress + ":" + this.infoPort); 
	    LOG.info("JobTracker webserver: " + this.infoServer.getPort());
	    
	    while (true) {
	    	try {
	    		// if we haven't contacted the namenode go ahead and do it
	    		if (fs == null) {
	    			fs = FileSystem.get(conf);
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
	    
	    /* Install the declarative definitions. */
	    try {
			install(context);
		} catch (UpdateException e) {
			throw new IOException(e);
		}
	}

	private static SimpleDateFormat getDateFormat() {
		return new SimpleDateFormat("yyyyMMddHHmm");
	}
	
	private void install(System context) throws UpdateException {
		this.context = context;
		this.context().catalog().register(new JobTable((jol.core.Runtime)context));
		this.context().catalog().register(new TaskAttemptTable((jol.core.Runtime)context));
		this.context().catalog().register(new TaskCreate(this));
		this.context().catalog().register(new TaskReportTable((jol.core.Runtime)context));
		this.context().catalog().register(new TaskTable((jol.core.Runtime)context));
		this.context().catalog().register(new TaskTrackerActionTable((jol.core.Runtime)context));
		this.context().catalog().register(new TaskTrackerErrorTable((jol.core.Runtime)context));
		this.context().catalog().register(new TaskTrackerTable((jol.core.Runtime)context));
		
		URL program = 
			ClassLoader.getSystemClassLoader().
				getResource("org/apache/hadoop/mapred/declarative/jobTracker.olg");
		this.context.install("hadoop", program);

	}
	
	@Override
	public System context() {
		return this.context;
	}
	
	@Override
	public void offerService() throws IOException, InterruptedException {
		this.server.start();

		synchronized (this) {
			state = State.RUNNING;
		}
		this.server.join();
	}
	
	@Override
	public void stopTracker() {
		if (this.infoServer != null) {
			LOG.info("Stopping infoServer");
			try {
				this.infoServer.stop();
			} catch (InterruptedException ex) {
				ex.printStackTrace();
			}
		}
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
	public int getInfoPort() {
		return this.infoPort;
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
	    
	    return new Tuple(jobid, 
	    		         jobConf.getJobName(),
	    		         jobConf.getUser(), 
	    		         jobFile.toString(),
	    		         java.lang.System.currentTimeMillis(),
	    		         jobConf.getJobPriority(),
	    		         url,
	    		         JobStatus.PREP,
	    		         0f, 0f, 0f);
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
		if (argv.length != 0) {
			java.lang.System.out.println("usage: JobTracker");
			java.lang.System.exit(-1);
		}

		try {
			JobTracker tracker = startTracker(new JobConf());
			tracker.offerService();
		} catch (Throwable e) {
			LOG.fatal(StringUtils.stringifyException(e));
			java.lang.System.exit(-1);
		}
	}


}
