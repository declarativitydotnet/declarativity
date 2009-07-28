package org.apache.hadoop.mapred.declarative.master;

import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import jol.core.JolSystem;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.Server;
import org.apache.hadoop.mapred.Counters;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobHistory;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobProfile;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.JobSubmissionProtocol;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.MRConstants;
import org.apache.hadoop.mapred.MapTaskStatus;
import org.apache.hadoop.mapred.ReduceTaskStatus;
import org.apache.hadoop.mapred.StatusHttpServer;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.TaskReport;
import org.apache.hadoop.mapred.TaskStatus;
import org.apache.hadoop.mapred.TaskTrackerStatus;
import org.apache.hadoop.mapred.TaskStatus.Phase;
import org.apache.hadoop.mapred.TaskStatus.State;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.Constants.TaskPhase;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
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
	private final static String[] programs = {"jobtracker", "scheduler", "policy"};

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
	
	// Used to provide an HTML view on Job, Task, and TaskTracker structures
	private StatusHttpServer infoServer;
	private int infoPort;

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

		/******************* Set ports, start RPC servers, etc. ***********/
		InetSocketAddress addr = getAddress(conf);
		this.localMachine = addr.getHostName();
		this.port = addr.getPort();
		int handlerCount = conf.getInt("mapred.job.tracker.handler.count", 10);
		this.masterInterface = new JobTrackerServer(context, this, conf);
		this.server = RPC.getServer(this.masterInterface,
				addr.getHostName(), addr.getPort(),
				handlerCount, false, conf);

		this.server.start();
		/***************************************************************/

		/******** The rpc/web-server ports can be ephemeral ports... 
	              ... ensure we have the correct info. ****************/
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
		this.context().catalog().register(new JobCompletionTable());
		this.context().catalog().register(new AssignTracker());
		this.context().catalog().register(new BestCandidate());
		this.context().catalog().register(new TaskTable(context));
		this.context().catalog().register(new TaskTrackerActionTable(context));
		this.context().catalog().register(new TaskTrackerErrorTable(context));
		this.context().catalog().register(new TaskTrackerTable(context, this));
		this.context().catalog().register(new NetworkTopologyTable(context, this));
		
		for (String program : programs) {
			URL p = ClassLoader.getSystemClassLoader().getResource(program + ".olg");
			this.context.install("jobTracker", p);
			this.context.evaluate();
		}

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
	public FileSystem fileSystem() {
		return this.fs;
	}

	@Override
	public Tuple newJob(JobID jobid) throws IOException {
		Path localJobFile = this.conf.getLocalPath(SUBDIR  +"/"+jobid + ".xml");
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

	///////////////////////////////////////////////////////
	// Accessors for objects that want info on jobs, tasks,
	// trackers, etc.
	///////////////////////////////////////////////////////
	@Override
	public int getTotalSubmissions() {
		Table table = this.context.catalog().table(JobTable.TABLENAME);
		return table.cardinality().intValue();
	}

	@Override
	public String getJobTrackerMachine() {
		return localMachine;
	}

	@Override
	public String getTrackerIdentifier() {
		return this.identifier;
	}

	@Override
	public int getTrackerPort() {
		return port;
	}

	@Override
	public int getInfoPort() {
		return infoPort;
	}

	@Override
	public TaskReport[] getMapTaskReports(JobID jobId) {
		TaskTable table =
			(TaskTable) this.context.catalog().table(TaskTable.TABLENAME);
		return table.mapReports(jobId);
	}

	@Override
	public TaskReport[] getReduceTaskReports(JobID jobId) {
		TaskTable table =
			(TaskTable) this.context.catalog().table(TaskTable.TABLENAME);
		return table.reduceReports(jobId);
	}

	@Override
	public int getNumResolvedTaskTrackers() {
		Table table = this.context.catalog().table(TaskTrackerTable.TABLENAME);
		return table.cardinality().intValue();
	}

	@Override
	public TaskStatus getTaskStatus(TaskAttemptID a) {
		try {
			Table table = this.context.catalog().table(TaskAttemptTable.TABLENAME);
			TupleSet result = table.primary().lookupByKey(a.getJobID(), a.getTaskID(), a.getId());
			for (Tuple t : result) {
				Float progress = (Float) t.value(TaskAttemptTable.Field.PROGRESS.ordinal());
				TaskState state = (TaskState) t.value(TaskAttemptTable.Field.STATE.ordinal());
				TaskPhase phase = (TaskPhase) t.value(TaskAttemptTable.Field.PHASE.ordinal());
				String tracker = (String) t.value(TaskAttemptTable.Field.TTLOCATION.ordinal());
				if (a.isMap()) {
					return new MapTaskStatus(a, progress, 
							TaskStatus.State.valueOf(state.name()), "FIXME",
							state.toString(), tracker, 
							TaskStatus.Phase.valueOf(phase.name()), new Counters());
				}
				else {
					return new ReduceTaskStatus(a, progress, 
							TaskStatus.State.valueOf(state.name()), "FIXME",
							state.toString(), tracker, 
							TaskStatus.Phase.valueOf(phase.name()), new Counters());
				}
			}
		} catch (Throwable t) {

		}
		return null;
	}

	@Override
	public TaskTrackerStatus getTaskTracker(String name) {
		try {
			Table table = this.context.catalog().table(TaskTrackerTable.TABLENAME);
			TupleSet result = table.primary().lookupByKey(name);
			for (Tuple t : result) {
				String host = (String) t.value(TaskTrackerTable.Field.HOST.ordinal());
				Integer port = (Integer) t.value(TaskTrackerTable.Field.HTTP_PORT.ordinal());
				Integer failures = (Integer) t.value(TaskTrackerTable.Field.FAILURES.ordinal());
				Integer maxmaps = (Integer) t.value(TaskTrackerTable.Field.MAX_MAP.ordinal());
				Integer maxred  = (Integer) t.value(TaskTrackerTable.Field.MAX_REDUCE.ordinal());
				return new TaskTrackerStatus(name, host, 
						port, new ArrayList<TaskStatus>(),
						failures, maxmaps, maxred); 
			}
		} catch (Throwable t) {

		}
		return null;

	}

	public JobStatus getJobStatus(JobID jobid) {
		try {
			return this.masterInterface.getJobStatus(jobid);
		} catch (Throwable t) {
		}
		return null;
	}

	public Set<TaskTrackerStatus> taskTrackers() {
		Set<TaskTrackerStatus> trackers = new HashSet<TaskTrackerStatus>();
		Table table = this.context.catalog().table(TaskTrackerTable.TABLENAME);
		for (Tuple t : table.tuples()) {
			String name = (String) t.value(TaskTrackerTable.Field.TTLOCATION.ordinal());
			String host = (String) t.value(TaskTrackerTable.Field.HOST.ordinal());
			Integer port = (Integer) t.value(TaskTrackerTable.Field.HTTP_PORT.ordinal());
			Integer failures = (Integer) t.value(TaskTrackerTable.Field.FAILURES.ordinal());
			Integer maxmaps = (Integer) t.value(TaskTrackerTable.Field.MAX_MAP.ordinal());
			Integer maxred  = (Integer) t.value(TaskTrackerTable.Field.MAX_REDUCE.ordinal());
			trackers.add(new TaskTrackerStatus(name, host,  port, 
					new ArrayList<TaskStatus>(), failures, maxmaps, maxred)); 
		}
		return trackers;
	}

	public JobSubmissionProtocol jobInterface() {
		return this.masterInterface;
	}

	public JobProfile getJobProfile(JobID jobid) {
		try {
			return this.masterInterface.getJobProfile(jobid);
		} catch (IOException e) {
			return null;
		}
	}
	
	public TupleSet createTasks(JobID jobid) throws IOException {
		TupleSet tasks = new BasicTupleSet();
		
		Path localJobFile = this.conf.getLocalPath(SUBDIR  +"/"+jobid + ".xml");
		JobConf jobConf   = new JobConf(localJobFile);

	    Path sysDir = systemDir();
	    FileSystem fs = sysDir.getFileSystem(jobConf);
	    DataInputStream splitFile = fs.open(new Path(jobConf.get("mapred.job.split.file")));

	    JobClient.RawSplit[] splits;
	    try {
	      splits = JobClient.readSplitFile(splitFile);
	    } finally {
	      splitFile.close();
	    }

	    if (jobConf.getNumMapTasks() != splits.length) {
	    	System.err.println("ERROR: MISMATCH IN MAP COUNT BETWEEN CONF AND SPLIT");
	    	jobConf.setNumMapTasks(splits.length);
	    }

	    int mapid = 0;
	    for(int i=0; i < splits.length; i++) {
	    	if (splits[i].getLocations().length == 0) {
	    		System.err.println("ERROR: SPLIT DOES NOT HAVE A LOCATION.");
	    		continue;
	    	}
	      tasks.add(create(jobid, TaskType.MAP, mapid++, splits[i], jobConf.getNumMapTasks()));
	    }

	    //
	    // Create reduce tasks
	    //
	    int numReduceTasks = jobConf.getNumReduceTasks();

	    for (int i = 0; i < numReduceTasks; i++) {
	    	tasks.add(create(jobid, TaskType.REDUCE, i, null, jobConf.getNumMapTasks()));
	    }

	    // create job specific temporary directory in output path
	    Path outputPath = FileOutputFormat.getOutputPath(jobConf);
	    if (outputPath != null) {
	      Path tmpDir = new Path(outputPath, MRConstants.TEMP_DIR_NAME);
	      FileSystem fileSys = tmpDir.getFileSystem(jobConf);
	      if (!fileSys.mkdirs(tmpDir)) {
	        throw new IOException("Mkdirs failed to create " + tmpDir.toString());
	      }
	    }

		return tasks;
	}

	private Tuple create(JobID jobid, TaskType type, Integer partition, 
			JobClient.RawSplit split, int mapCount) {
		TaskID taskid = new TaskID(jobid, type == TaskType.MAP, partition);

		return new Tuple(jobid, taskid, type, partition, split, mapCount, 
				         new org.apache.hadoop.mapred.declarative.util.TaskState(jobid, taskid));
	}


	////////////////////////////////////////////////////////////
	// main()
	////////////////////////////////////////////////////////////

	/**
	 * Start the JobTracker process.  This is used only for debugging.  As a rule,
	 * JobTracker should be run as part of the DFS Namenode process.
	 */
	public static void main(String argv[]) throws IOException, InterruptedException {
		StringUtils.startupShutdownMessage(JobTracker.class, argv, LOG);
		if (argv.length != 0) {
			System.out.println("usage: JobTracker");
			System.exit(-1);
		}

		try {
			JobTracker tracker = startTracker(new JobConf());
			tracker.offerService();
		} catch (Throwable e) {
			LOG.fatal(StringUtils.stringifyException(e));
			System.exit(-1);
		}
	}

}
