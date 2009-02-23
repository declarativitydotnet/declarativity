package org.apache.hadoop.mapred.declarative.table;

import java.io.DataInputStream;

import java.io.IOException;
import java.util.List;

import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.MRConstants;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.declarative.Constants.TaskType;
import org.apache.hadoop.mapred.declarative.util.FileInput;
import org.apache.hadoop.mapred.declarative.util.TaskState;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.Function;


public class TaskCreate extends Function {
	/** An enumeration of all fields. */
	public enum Field{JOBID, JOBCONF, JOBFILE, TASKID, TYPE,
		              PARTITION, FILE_INPUT, MAP_COUNT, STATUS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,     // Job identifier
		JobConf.class,   // JobConf
		String.class,    // JobFile
		TaskID.class,    // Task identifier
		TaskType.class,  // Task type
		Integer.class,   // Partition number
		FileInput.class, // Input file
		Integer.class,   // Map count
		TaskState.class // Task status
	};
	
	private JobTracker jobTracker;

	public TaskCreate(JobTracker jobTracker) {
		super("taskCreate", SCHEMA);
		this.jobTracker = jobTracker;
	}

	@Override
	public TupleSet insert(TupleSet insertions, TupleSet conflicts) throws UpdateException {
		TupleSet tasks = new TupleSet(name());
		for (Tuple tuple : insertions) {
			JobID jobid    = (JobID) tuple.value(Field.JOBID.ordinal());
			JobConf conf   = (JobConf) tuple.value(Field.JOBCONF.ordinal());
			String jobFile = (String) tuple.value(Field.JOBFILE.ordinal());
			
			if (jobid == null || conf == null) {
				throw new UpdateException("Error: table function " + name() + 
						                  " requires valid jobid and jobconf!");
			}
			try {
				tasks.addAll(createTasks(jobid, conf, jobFile));
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
	    Path[] paths = FileInputFormat.getInputPaths(conf);
	    
	    JobClient.RawSplit[] splits;
	    try {
	      splits = JobClient.readSplitFile(splitFile);
	    } finally {
	      splitFile.close();
	    }
	    
	    int numMapTasks = 0;
	    for (int i = 0; i < splits.length; i++) {
	    	if (splits[i] != null && splits[i].getLocations().length > 0) {
	    		numMapTasks++;
	    	}
	    }
	    
	    if (conf.getNumMapTasks() != splits.length) {
	    	System.err.println("ERROR: MISMATCH IN MAP COUNT BETWEEN CONF AND SPLIT");
	    	conf.setNumMapTasks(splits.length);
	    }
	    
	    int mapid = 0;
	    for(int i=0; i < splits.length; i++) {
	    	if (splits[i].getLocations().length == 0) {
	    		System.err.println("ERROR: SPLIT DOES NOT HAVE A LOCATION.");
	    		continue;
	    	}
	      tasks.add(create(jobid, TaskType.MAP, conf, jobFile, mapid++, new FileInput(null, splits[i]), numMapTasks));
	    }
	    
	    //
	    // Create reduce tasks
	    //
	    int numReduceTasks = conf.getNumReduceTasks();

	    for (int i = 0; i < numReduceTasks; i++) {
	    	tasks.add(create(jobid, TaskType.REDUCE, conf, jobFile, i, null, numMapTasks));
	    }
	    
	    // create job specific temporary directory in output path
	    Path outputPath = FileOutputFormat.getOutputPath(conf);
	    if (outputPath != null) {
	      Path tmpDir = new Path(outputPath, MRConstants.TEMP_DIR_NAME);
	      FileSystem fileSys = tmpDir.getFileSystem(conf);
	      if (!fileSys.mkdirs(tmpDir)) {
	        throw new IOException("Mkdirs failed to create " + tmpDir.toString());
	      }
	    }

		return tasks;
	}
	
	private Tuple create(JobID jobid, TaskType type, JobConf conf, String jobFile, 
			             Integer partition, FileInput input, int mapCount) {
		TaskID taskid = new TaskID(jobid, type == TaskType.MAP, partition);
		
		return new Tuple(jobid, conf, jobFile, 
				         taskid, type, partition, input, 
				         mapCount, new TaskState(jobid, taskid));
	}
	
}
