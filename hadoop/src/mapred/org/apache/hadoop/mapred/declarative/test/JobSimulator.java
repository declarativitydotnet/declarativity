package org.apache.hadoop.mapred.declarative.test;

import java.io.IOException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Random;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobPriority;
import org.apache.hadoop.mapred.JobTracker;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Partitioner;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.RunningJob;
import org.apache.hadoop.mapred.declarative.JobTrackerImpl;
import org.apache.hadoop.mapred.lib.NullOutputFormat;
import org.apache.hadoop.util.Tool;

public class JobSimulator extends Thread {
	private static class Job extends Configured 
	implements Runnable, Tool, 
			   Mapper<IntWritable, IntWritable, IntWritable, IntWritable>,  
			   Reducer<IntWritable, IntWritable, IntWritable, IntWritable>,  
			   Partitioner<IntWritable, IntWritable> {
		private String name;
		private int maps;
		private int reduces;
		private Path input;
		private JobPriority priority;
		
		public Job(JobPriority priority, String name, int maps, int reduces, Path input) {
			this.name = name;
			this.maps = maps;
			this.reduces = reduces;
			this.input = input;
			this.priority = priority;
		}

		public void run() {
			try {
				JobConf job = new JobConf();
				job.setJobPriority(this.priority);
				job.setMapperClass(Job.class);
				job.setReducerClass(Job.class);
			    job.setCombinerClass(Job.class);
				
				System.err.println("SETTING MAPS TO " + maps);
				job.setNumMapTasks(maps);
				job.setNumReduceTasks(reduces);
				
				job.setMapOutputKeyClass(IntWritable.class);
				job.setMapOutputValueClass(IntWritable.class);
				job.setOutputFormat(NullOutputFormat.class);
				
				// job.setInputFormat(SequenceFileInputFormat.class);
				job.setSpeculativeExecution(false);
				job.setJobName(name);
			    FileInputFormat.setInputPaths(job, input); 
				// FileInputFormat.addInputPath(job, input);

				RunningJob runner = null;
				while (runner == null) {
					try {
						runner = JobClient.runJob(job);
					} catch (Throwable t) {sleep(100);}
				}
				while (!runner.isComplete()) {
					sleep(1000);
				}
				System.err.println("JOB " + name + " completed " + 
						(runner.isSuccessful() ? "successfully!" : "unsuccessfully!"));
			} catch (IOException e) {
				e.printStackTrace();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		public void map(IntWritable key, IntWritable value,
				OutputCollector<IntWritable, IntWritable> output,
				Reporter reporter) throws IOException {
			System.err.println("MAP EXECUTION");
		}

		public void close() throws IOException {
		}

		public void reduce(IntWritable key, Iterator<IntWritable> values,
				OutputCollector<IntWritable, IntWritable> output,
				Reporter reporter) throws IOException {
			System.err.println("REDUCE EXECUTION");
		}

		public int getPartition(IntWritable key, IntWritable value, int numPartitions) {
			return key.get() % numPartitions;
		}

		public int run(String[] args) throws Exception {
			return 0;
		}

		public void configure(JobConf job) {
		}
	}

	private Set<Job> jobs;
	
	private Executor executor;
	
	public JobSimulator() {
		this.jobs = new HashSet<Job>();
		this.executor = Executors.newCachedThreadPool();
	}
	
	public void run() {
		int index = 1;
		JobConf  conf = new JobConf();
		try {
			while (!isInterrupted()) {
				try {
					sleep(1000);
					Random random = new Random();
					Path tempPath = new Path("/tmp/job.simulator.data" + random.nextInt(5));
					FileSystem fs = FileSystem.get(conf);
					SequenceFile.Writer writer = 
						SequenceFile.createWriter(fs, conf, tempPath, 
								IntWritable.class, IntWritable.class);
					writer.close();
					String name = "job" + jobs.size();
					JobPriority[] priorities = JobPriority.values();
					Job job = new Job(priorities[random.nextInt(priorities.length)],
							          "job"+jobs.size(), 200, 10, tempPath);
					jobs.add(job);
					this.executor.execute(job);
					if (index-- <= 0) return;
				} catch (InterruptedException e) { }
			}
		} catch (Throwable t) {
			t.printStackTrace();
		}
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
			JobTrackerImpl tracker = (JobTrackerImpl) JobTracker.startTracker(new JobConf());
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
			e.printStackTrace();
			java.lang.System.exit(-1);
		}
	}
}
