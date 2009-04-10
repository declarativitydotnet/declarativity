package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobPriority;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.declarative.util.JobState;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.table.BasicTable;
import jol.types.table.Key;
import jol.types.table.TableName;

public class JobTable extends BasicTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName("hadoop", "job");
	public static final TableName INIT = new TableName("hadoop", "initJob");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all clock table fields. */
	public enum Field{JOBID, JOBNAME, JOBFILE, JOBCONF, USER, URL, PRIORITY, SUBMIT_TIME, FINISH_TIME, STATUS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,        // JobID
		String.class,       // Name
		String.class,       // File
		JobConf.class,      // JobConf
		String.class,       // User
		String.class,       // URL
		JobPriority.class,  // Priority
		Long.class,         // Submit time
		Long.class,         // Finish time
		JobState.class      // Job status
	};
	
	public JobTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}
	
	public static Tuple tuple(JobID jobid, String jobFile, JobConf conf, String url) {
		return new Tuple(jobid, 
				         conf.getJobName(), 
				         jobFile,
				         conf,
				         conf.getUser(), 
				         url, 
				         conf.getJobPriority(),
				         java.lang.System.currentTimeMillis(),
				         0L,
		                 new JobState(jobid, conf));
		
	}

}
