package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.JobStatus;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.declarative.util.JobState;
import org.apache.hadoop.mapred.declarative.util.Wrapper;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class JobTable extends ObjectTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName(JobTracker.PROGRAM, "job");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all clock table fields. */
	public enum Field{JOBID, NAME, USER, FILE, JOBCONF, URL, PRIORITY, MAPS, REDUCES, STATUS};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		JobID.class,     // JobID
		String.class,    // Name
		String.class,    // User
		String.class,    // File
		Wrapper.class,   // JobConf
		String.class,    // URL
		Enum.class,      // Priority
		Integer.class,   // Map count
		Integer.class,   // Reduce count
		JobState.class   // Job status
	};
	
	private Runtime context;

	public JobTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		this.context = context;
	}
	
	public static Tuple tuple(JobID jobid, String jobFile, JobConf conf, String url) {
		return new Tuple(jobid, 
				         conf.getJobName(), 
				         conf.getUser(), 
				         jobFile,
				         new Wrapper<JobConf>(conf),
				         url, 
				         conf.getJobPriority(),
				         0, 0,
		                 new JobState(jobid));
		
	}

}
