package org.apache.hadoop.mapred.declarative.table;

import java.io.DataInputStream;

import java.io.IOException;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.util.TaskState;
import org.apache.hadoop.mapred.declarative.util.Wrapper;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.table.Function;


public class TaskCreate extends Function {
	/** An enumeration of all fields. */
	public enum Field{JOBID, JOBCONF, JOBFILE, TASKID, TYPE,
		              PARTITION, SPLIT_FILE, MAP_COUNT, STATUS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,     // Job identifier
		Wrapper.class,   // JobConf
		String.class,    // JobFile
		TaskID.class,    // Task identifier
		Enum.class,      // Task type
		Integer.class,   // Partition number
		Wrapper.class,   // Split file
		Integer.class,   // Map count
		TaskState.class // Task status
	};
	
	private JobTracker jobTracker;

	public TaskCreate(JobTracker jobTracker) {
		super("taskCreate", new TypeList(SCHEMA));
		this.jobTracker = jobTracker;
	}

	@Override
	public TupleSet insert(TupleSet insertions, TupleSet conflicts) throws UpdateException {
		TupleSet tasks = new TupleSet(name());
		for (Tuple tuple : insertions) {
			JobID jobid           = (JobID) tuple.value(Field.JOBID.ordinal());
			Wrapper<JobConf> conf = (Wrapper<JobConf>) tuple.value(Field.JOBCONF.ordinal());
			String jobFile        = (String) tuple.value(Field.JOBFILE.ordinal());
			
			if (jobid == null || conf == null) {
				throw new UpdateException("Error: table function " + name() + 
						                  " requires valid jobid and jobconf!");
			}
			try {
				tasks.addAll(createTasks(jobid, conf.object(), jobFile));
			} catch (IOException e) {
				throw new UpdateException(e.toString());
			}
		}
		return tasks;
	}
	
	private TupleSet createTasks(JobID jobid, JobConf conf, String jobFile) throws IOException {
		TupleSet tasks = new TupleSet(name());
		
	    Path sysDir = this.jobTracker.systemDir();
	    FileSystem fs = sysDir.getFileSystem(conf);
	    DataInputStream splitFile = fs.open(new Path(conf.get("mapred.job.split.file")));
	    
	    JobClient.RawSplit[] splits;
	    try {
	      splits = JobClient.readSplitFile(splitFile);
	    } finally {
	      splitFile.close();
	    }
	    
	    int numMapTasks = splits.length;
	    
	    for(int i=0; i < numMapTasks; ++i) {
	      tasks.add(create(jobid, TaskType.MAP, conf, jobFile, i, splits[i], numMapTasks));
	    }
	    
	    //
	    // Create reduce tasks
	    //
	    int numReduceTasks = conf.getNumReduceTasks();

	    for (int i = 0; i < numReduceTasks; i++) {
	    	tasks.add(create(jobid, TaskType.REDUCE, conf, jobFile, i, null, numMapTasks));
	    }

		return tasks;
	}
	
	private Tuple create(JobID jobid, TaskType type, JobConf conf, String jobFile, 
			             Integer partition, JobClient.RawSplit split, int mapCount) {
		TaskID taskid = new TaskID(jobid, type == TaskType.MAP, partition);
		
		return new Tuple(jobid, new Wrapper<JobConf>(conf), jobFile, 
				         taskid, type, partition, 
				         new Wrapper<JobClient.RawSplit>(split), 
				         mapCount, new TaskState(jobid, taskid));
	}
	
}
