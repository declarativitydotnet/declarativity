package org.apache.hadoop.mapred.declarative.slave;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.regex.Pattern;

import jol.core.JolSystem;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileUtil;
import org.apache.hadoop.fs.LocalDirAllocator;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.MRConstants;
import org.apache.hadoop.mapred.ReduceTask;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskCompletionEvent;
import org.apache.hadoop.mapred.TaskRunner;
import org.apache.hadoop.mapred.TaskStatus;
import org.apache.hadoop.mapred.TaskTracker;
import org.apache.hadoop.mapred.TaskTrackerAction;
import org.apache.hadoop.mapred.TaskUmbilicalProtocol;
import org.apache.hadoop.mapred.declarative.Constants;
import org.apache.hadoop.mapred.declarative.Constants.JobState;
import org.apache.hadoop.mapred.declarative.Constants.TaskState;
import org.apache.hadoop.mapred.declarative.table.JobCompletionTable;
import org.apache.hadoop.mapred.declarative.table.MapCompletionTable;
import org.apache.hadoop.mapred.declarative.table.TaskAttemptTable;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerActionTable;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerErrorTable;
import org.apache.hadoop.mapred.declarative.table.TaskTrackerTable;
import org.apache.hadoop.metrics.Updater;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;
import org.apache.hadoop.util.RunJar;
import org.apache.hadoop.util.StringUtils;

public class TaskTrackerImpl extends TaskTracker implements Runnable {
	private final static String[] programs = {"tasktracker"};
	private final static String PROGRAM = "hadoop";
	
	private Path systemDir;
	
	private jol.core.Runtime jollib;
	
	private Map<TaskAttemptID, TaskRunner> tasks;
	
	private Map<JobID, ArrayList<TaskCompletionEvent>> mapCompletionEvents;
	
	private TaskAttemptTable attemptTable;
	
	public TaskTrackerImpl(JobConf conf) throws IOException {
		super(conf, MapOutputServlet.class);
		this.tasks = new HashMap<TaskAttemptID, TaskRunner>();
		this.mapCompletionEvents = new HashMap<JobID, ArrayList<TaskCompletionEvent>>();
		FileSystem fs  = FileSystem.get(conf);
		this.systemDir = new Path(conf.get("mapred.system.dir", "/tmp/hadoop/mapred/system"));
		this.systemDir = fs.makeQualified(this.systemDir);
	}
	
	public static int getJolPort(Configuration conf) {
		return conf.getInt("mapred.jol.port", 9002);
	}
	
	@Override
	public synchronized void initialize() throws IOException {
		super.initialize();
		if (this.jollib != null) {
			this.jollib.shutdown();
		}
		
		try {
			this.jollib = (jol.core.Runtime)
				jol.core.Runtime.create(jol.core.Runtime.DEBUG_WATCH, System.err, 0);
		} catch (JolRuntimeException e) {
			throw new IOException(e);
		}
		
		/* Install tables relevant to the task tracker. */
		this.attemptTable = new TaskAttemptTable((jol.core.Runtime)jollib);
		this.jollib.catalog().register((Table)this.attemptTable);
		
		this.jollib.catalog().register(new TaskTrackerTable( (jol.core.Runtime)jollib));
		this.jollib.catalog().register(new TaskTrackerErrorTable( (jol.core.Runtime)jollib));
		this.jollib.catalog().register(new TaskTrackerActionTable((jol.core.Runtime)jollib));
		
		Table jobCompletion = new JobCompletionTable();
		Table mapCompletion = new MapCompletionTable((jol.core.Runtime)jollib);
		this.jollib.catalog().register(mapCompletion);
		this.jollib.catalog().register(jobCompletion);
		
		try {
			for (String program : programs) {
				URL p = ClassLoader.getSystemClassLoader().getResource(program + ".olg");
				this.jollib.install("taskTracker", p);
				this.jollib.evaluate();
			}
		} catch (JolRuntimeException e) {
			throw new IOException (e);
		}
		
		jobCompletion.register(new Callback() {
			public void deletion(TupleSet tuples) { }
			public void insertion(TupleSet tuples) {
				synchronized (mapCompletionEvents) {
					for (Tuple t : tuples) {
						JobID    jobid = (JobID) t.value(JobCompletionTable.Field.JOBID.ordinal());
						JobState state = (JobState) t.value(JobCompletionTable.Field.STATE.ordinal());
						mapCompletionEvents.remove(jobid);
					}
				}
			}
		
		});
		
		mapCompletion.register(new Callback() {
			public void deletion(TupleSet tuples) { }
			public void insertion(TupleSet tuples) {
				synchronized (mapCompletionEvents) {
					for (Tuple t : tuples) {
						JobID      jobid = (JobID)         t.value(MapCompletionTable.Field.JOBID.ordinal());
						TaskAttemptID id = (TaskAttemptID) t.value(MapCompletionTable.Field.ATTEMPTID.ordinal());
						TaskState  state = (TaskState)     t.value(MapCompletionTable.Field.STATE.ordinal());
						String   fileLoc = (String)        t.value(MapCompletionTable.Field.FILELOCATION.ordinal());

						if (state == TaskState.SUCCEEDED) {
							if (!mapCompletionEvents.containsKey(jobid)) {
								mapCompletionEvents.put(jobid, new ArrayList<TaskCompletionEvent>());
							}
							int eventID = mapCompletionEvents.get(jobid).size();
							TaskCompletionEvent event = new TaskCompletionEvent(eventID,
									id, id.getTaskID().getId(), id.isMap(),
									TaskCompletionEvent.Status.valueOf(state.name()),
									fileLoc);
							mapCompletionEvents.get(jobid).add(event);
						}
					}
				}
			}
		});
		
		try {
			InetSocketAddress jtJolAddr = JobTracker.getJolAddress(this.fConf);
			String jtJolAddress = "tcp:" + jtJolAddr.getHostName() + ":" + jtJolAddr.getPort();
			String ttJolAddress = "tcp:" + this.localHostname + ":" + this.jollib.getServerPort();
			
			/* Install configuration state */
			TupleSet config = new BasicTupleSet();
			config.add(new Tuple(ttJolAddress, this.localHostname,  this.httpPort, 
					             this.maxCurrentMapTasks, this.maxCurrentReduceTasks, this));
			this.jollib.schedule(PROGRAM, new TableName(PROGRAM, "configuration"), config, null);
			this.jollib.evaluate();
			
			TupleSet jtState = new BasicTupleSet();
			jtState.add(new Tuple(ttJolAddress, jtJolAddress, Constants.TrackerState.INITIAL));
			this.jollib.schedule(PROGRAM, new TableName(PROGRAM, "jobTracker"), jtState, null);
			this.jollib.evaluate();
		} catch (JolRuntimeException e) {
			throw new IOException(e);
		}
	}
	
	public void kill(TaskRunner runner) {
		// TODO
	}
	
    /**
     * Kick off the task execution
     */
	public synchronized TaskRunner launch(Task task) {
		if (!localize(task)) return null;
		try {
			TaskRunner runner = task.createRunner(this);
			this.tasks.put(task.getTaskID(), runner);
			runner.start();
			return runner;
		} catch (IOException e) {
			return null;
		}
	}
    
    // intialize the job directory
	private boolean localize(Task task) {
		try {
			JobID jobId = task.getJobID();
			Path jobFile = new Path(task.getJobFile());
			// Get sizes of JobFile and JarFile
			// sizes are -1 if they are not present.
			FileSystem fs = systemDir.getFileSystem(fConf);
			FileStatus status = null;
			long jobFileSize = -1;
			try {
				status = fs.getFileStatus(jobFile);
				jobFileSize = status.getLen();
			} catch(FileNotFoundException fe) {
				jobFileSize = -1;
			}
			Path localJobFile = lDirAlloc.getLocalPathForWrite((getJobCacheSubdir()
					+ Path.SEPARATOR + jobId 
					+ Path.SEPARATOR + "job.xml"),
					jobFileSize, fConf);

			Path jobDir = localJobFile.getParent();
			FileSystem localFs = FileSystem.getLocal(fConf);

			JobConf localJobConf = new JobConf(localJobFile);
			if (!localFs.exists(localJobFile)){
				fs.copyToLocalFile(jobFile, localJobFile);
			}

			// create the 'work' directory
			// job-specific shared directory for use as scratch space 
			Path workDir = lDirAlloc.getLocalPathForWrite((getJobCacheSubdir()
					+ Path.SEPARATOR + jobId 
					+ Path.SEPARATOR + "work"), fConf);
			if (!localFs.exists(workDir) && !localFs.mkdirs(workDir)) {
				throw new IOException("Mkdirs failed to create "  + workDir.toString());
			}
			System.setProperty("job.local.dir", workDir.toString());
			localJobConf.set("job.local.dir", workDir.toString());

			// copy Jar file to the local FS and unjar it.
			String jarFile = localJobConf.getJar();
			long jarFileSize = -1;
			if (jarFile != null) {
				Path jarFilePath = new Path(jarFile);
				try {
					status = fs.getFileStatus(jarFilePath);
					jarFileSize = status.getLen();
				} catch(FileNotFoundException fe) {
					jarFileSize = -1;
				}
				// Here we check for and we check five times the size of jarFileSize
				// to accommodate for unjarring the jar file in work directory 
				Path localJarFile = new Path(lDirAlloc.getLocalPathForWrite(
						getJobCacheSubdir()
						+ Path.SEPARATOR + jobId 
						+ Path.SEPARATOR + "jars",
						5 * jarFileSize, fConf), "job.jar");
				if (!localFs.exists(localJarFile)) {
					if (!localFs.mkdirs(localJarFile.getParent())) {
						throw new IOException("Mkdirs failed to create jars directory "); 
					}
					fs.copyToLocalFile(jarFilePath, localJarFile);
					localJobConf.setJar(localJarFile.toString());
					OutputStream out = localFs.create(localJobFile);
					try {
						localJobConf.write(out);
					} finally {
						out.close();
					}
					// also unjar the job.jar files 
					RunJar.unJar(new File(localJarFile.toString()),
							new File(localJarFile.getParent().toString()));
				}
			}
			localizeTask(task, localJobConf);
		}
		catch (IOException e) {
			e.printStackTrace();
			purge(task.getJobID());
			return false;
		}
		return true;
	}

    private void localizeTask(Task task, JobConf localJobConf) throws IOException{
    	Path localTaskDir = 
    		lDirAlloc.getLocalPathForWrite((TaskTracker.getJobCacheSubdir() + 
    				Path.SEPARATOR + task.getJobID() + Path.SEPARATOR +
    				task.getTaskID()), fConf);

    	FileSystem localFs = FileSystem.getLocal(fConf);
    	if (!localFs.mkdirs(localTaskDir)) {
    		throw new IOException("Mkdirs failed to create " 
    				+ localTaskDir.toString());
    	}

    	// create symlink for ../work if it already doesnt exist
    	String workDir = lDirAlloc.getLocalPathToRead(
    			TaskTracker.getJobCacheSubdir() 
    			+ Path.SEPARATOR + task.getJobID() 
    			+ Path.SEPARATOR  
    			+ "work", fConf).toString();
    	String link = localTaskDir.getParent().toString() + Path.SEPARATOR + "work";
    	File flink = new File(link);
    	if (!flink.exists()) FileUtil.symLink(workDir, link);

    	// create the working-directory of the task 
    	Path cwd = lDirAlloc.getLocalPathForWrite(
    			TaskTracker.getJobCacheSubdir() 
    			+ Path.SEPARATOR + task.getJobID() 
    			+ Path.SEPARATOR + task.getTaskID()
    			+ Path.SEPARATOR + MRConstants.WORKDIR,
    			fConf);
    	if (!localFs.mkdirs(cwd)) {
    		throw new IOException("Mkdirs failed to create " 
    				+ cwd.toString());
    	}

    	Path localTaskFile = new Path(localTaskDir, "job.xml");
    	task.setJobFile(localTaskFile.toString());
    	localJobConf.set("mapred.local.dir", fConf.get("mapred.local.dir"));

    	localJobConf.set("mapred.task.id", task.getTaskID().toString());

    	task.localizeConfiguration(localJobConf);

    	List<String[]> staticResolutions = NetUtils.getAllStaticResolutions();
    	if (staticResolutions != null && staticResolutions.size() > 0) {
    		StringBuffer str = new StringBuffer();

    		for (int i = 0; i < staticResolutions.size(); i++) {
    			String[] hostToResolved = staticResolutions.get(i);
    			str.append(hostToResolved[0]+"="+hostToResolved[1]);
    			if (i != staticResolutions.size() - 1) {
    				str.append(',');
    			}
    		}
    		localJobConf.set("hadoop.net.static.resolutions", str.toString());
    	}
    	OutputStream out = localFs.create(localTaskFile);
    	try {
    		localJobConf.write(out);
    	} finally {
    		out.close();
    	}
    	task.setConf(localJobConf);
    }
    
    
	/**
	 * The task tracker is done with this job, so we need to clean up.
	 * @param action The action with the job
	 * @throws IOException
	 */
    private synchronized boolean purge(JobID jobid) {
    	try {
    		Path [] paths = getLocalFiles(fConf, 
    				getJobCacheSubdir() + Path.SEPARATOR +  jobid);
    		for (Path path : paths) {
    			FileSystem fs = path.getFileSystem(fConf);
    			fs.delete(path, true);
    		}
    		return true;
    	} catch (IOException e) {
    		return false;
    	}
    }      

	// get the full paths of the directory in all the local disks.
	private Path[] getLocalFiles(JobConf conf, String subdir) throws IOException{
		String[] localDirs = conf.getLocalDirs();
		Path[] paths = new Path[localDirs.length];
		FileSystem localFs = FileSystem.getLocal(conf);
		for (int i = 0; i < localDirs.length; i++) {
			paths[i] = new Path(localDirs[i], subdir);
			paths[i] = paths[i].makeQualified(localFs);
		}
		return paths;
	}



	@Override
	public Updater getTaskTrackerMetrics() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void run() {
		try {
			initialize();
			this.jollib.start();
			this.jollib.driver().join();
		} catch (Throwable t) {
			t.printStackTrace();
		} finally {
			if (this.jollib != null) {
				this.jollib.shutdown();
			}
		}
	}

	@Override
	public void done(TaskAttemptID taskid, boolean shouldBePromoted)
			throws IOException {
		Constants.TaskState state = shouldBePromoted ? 
				Constants.TaskState.COMMIT_PENDING : Constants.TaskState.SUCCEEDED;
		Constants.TaskPhase phase = taskid.isMap() ?
				Constants.TaskPhase.MAP : Constants.TaskPhase.REDUCE;
		
		TupleSet update = new BasicTupleSet();
		update.add(new Tuple(taskid, 1.0f, state, phase, "")); 
		try {
			this.jollib.schedule(PROGRAM, new TableName(PROGRAM, "statusUpdate"), update, null);
		} catch (Throwable t) {
			t.printStackTrace();
		}
	}

	@Override
	public void fsError(TaskAttemptID taskId, String message)
			throws IOException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public TaskCompletionEvent[] getMapCompletionEvents(JobID jobId, int fromIndex, int maxLocs) throws IOException {
		TaskCompletionEvent[] retevents = TaskCompletionEvent.EMPTY_ARRAY;
		synchronized(mapCompletionEvents) {
			if (mapCompletionEvents.containsKey(jobId)) {
				ArrayList<TaskCompletionEvent> events = mapCompletionEvents.get(jobId);
				if (events.size() > fromIndex) {
					int actualMax = Math.min(maxLocs, (events.size() - fromIndex));
					retevents = 
						events.subList(fromIndex, actualMax + fromIndex).
						toArray(new TaskCompletionEvent[actualMax]);
				}
			}
		}
		return retevents;
	}

	@Override
	public Task getTask(TaskAttemptID taskid) throws IOException {
		if (this.tasks.containsKey(taskid)) {
			return this.tasks.get(taskid).getTask();
		}
		throw new IOException("Unknown task id " + taskid + ".");
	}

	@Override
	public boolean ping(TaskAttemptID taskid) throws IOException {
		return this.tasks.containsKey(taskid);
	}

	@Override
	public void reportDiagnosticInfo(TaskAttemptID taskid, String trace)
			throws IOException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void shuffleError(TaskAttemptID taskId, String message)
			throws IOException {
		System.err.println("SHUFFLE ERROR " + message);
	}

	@Override
	public boolean statusUpdate(TaskAttemptID taskId, TaskStatus taskStatus)
			throws IOException, InterruptedException {
		if (this.tasks.containsKey(taskId)) {
			TupleSet update = new BasicTupleSet();
			update.add(new Tuple(taskId, taskStatus.getProgress(), 
					    Constants.TaskState.valueOf(taskStatus.getRunState().name()),
					    Constants.TaskPhase.valueOf(taskStatus.getPhase().name()),
					    taskStatus.getDiagnosticInfo()));
			try {
				this.jollib.schedule(PROGRAM, new TableName(PROGRAM, "statusUpdate"), update, null);
			} catch (Throwable t) {
				System.err.println("JOLLIB " + this.jollib);
				t.printStackTrace();
				return false;
			}
			return true;
		}
		return false;
	}

	@Override
	public long getProtocolVersion(String protocol, long clientVersion)
			throws IOException {
		if (protocol.equals(TaskUmbilicalProtocol.class.getName())) {
			return TaskUmbilicalProtocol.versionID;
		}
		throw new IOException("Unknown protocol " + protocol);
	}
	
	@Override
	public void reportTaskFinished(TaskAttemptID taskid) {
		this.tasks.remove(taskid);
		this.mapCompletionEvents.remove(taskid);
	}
	
	/**
	 * Start the TaskTracker, point toward the indicated JobTracker
	 */
	public static void main(String argv[]) throws Exception {
		StringUtils.startupShutdownMessage(TaskTracker.class, argv, LOG);
		if (argv.length != 0) {
			System.out.println("usage: TaskTracker");
			System.exit(-1);
		}
		try {
			JobConf conf=new JobConf();
			// enable the server to track time spent waiting on locks
			ReflectionUtils.setContentionTracing
			(conf.getBoolean("tasktracker.contention.tracking", false));
			new TaskTrackerImpl(conf).run();
		} catch (Throwable e) {
			LOG.error("Can not start task tracker because "+
					StringUtils.stringifyException(e));
			System.exit(-1);
		}
	}



}
