package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobPriority;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.declarative.util.JobState;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.basic.Wrapper;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class JobTable extends ObjectTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "job");
	public static final TableName INIT = new TableName(JobTracker.PROGRAM, "initJob");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all clock table fields. */
	public enum Field{JOBID, JOBNAME, JOBFILE, JOBCONF, USER, URL, PRIORITY, SUBMIT_TIME, STATUS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,        // JobID
		String.class,       // Name
		String.class,       // File
		Wrapper.class,      // JobConf
		String.class,       // User
		String.class,       // URL
		JobPriority.class,  // Priority
		Long.class,         // Submit time
		JobState.class      // Job status
	};
	
	public JobTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	public static Tuple tuple(JobID jobid, String jobFile, JobConf conf, String url) {
		return new Tuple(jobid, 
				         conf.getJobName(), 
				         jobFile,
				         new Wrapper<JobConf>(conf),
				         conf.getUser(), 
				         url, 
				         conf.getJobPriority(),
				         java.lang.System.currentTimeMillis(),
		                 new JobState(jobid));
		
	}

}
