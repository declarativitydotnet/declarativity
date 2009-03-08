package org.apache.hadoop.mapred.declarative.test;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
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
		private RunningJob runner;
		
		public Job(JobPriority priority, String name, int maps, int reduces, Path input) {
			this.name = name;
			this.maps = maps;
			this.reduces = reduces;
			this.input = input;
			this.priority = priority;
			this.runner = null;
		}
		
		public boolean isComplete() throws IOException {
			return this.runner == null || this.runner.isComplete();
		}
		
		public boolean isSuccessful() throws IOException {
			return this.runner == null || this.runner.isSuccessful();
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

				while (runner == null) {
					try {
						runner = JobClient.runJob(job);
					} catch (Throwable t) {sleep(100);}
				}
				while (!runner.isComplete()) {
					sleep(5000);
				}
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

	private TaskTrackerCluster cluster;
	
	private Set<Job> jobs;
	
	private Executor executor;
	
	private int jobCount;
	
	private int maps;
	
	private int reduces;
	
	public JobSimulator(int clusterSize, int jobs, int maps, int reduces) throws IOException {
		this(clusterSize, jobs, maps, reduces, null, null);
	}
	
	public JobSimulator(int clusterSize, int jobs, int maps, int reduces, OutputStream utilityLog, OutputStream callLog) throws IOException {
		cluster = new TaskTrackerCluster(new JobConf(), clusterSize, utilityLog, callLog);
		this.jobCount = jobs;
		this.maps = maps;
		this.reduces = reduces;
		this.jobs = new HashSet<Job>();
		this.executor = Executors.newCachedThreadPool();
	}
	
	
	public void run() {
		JobConf  conf = new JobConf();
		try {
			for (int i = 0; i < this.jobCount && !isInterrupted(); i++) {
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
							          "job"+jobs.size(), this.maps, this.reduces, tempPath);
					jobs.add(job);
					this.executor.execute(job);
				} catch (InterruptedException e) { }
			}
		} catch (Throwable t) {
			t.printStackTrace();
		}
		finally {
			cluster.start();
			while (jobs.size() > 0) {
				try { sleep(10000);
				} catch (InterruptedException e1) { }
				Set<Job> finished = new HashSet<Job>();
				for (Job j : jobs) {
					try {
						if (j.isComplete()) {
							finished.add(j);
						}
					} catch (IOException e) {
						finished.add(j);
					}
				}
				jobs.removeAll(finished);
			}
			cluster.stop();
			System.err.println("SIMULATION COMPLETE");
		}
	}
	
	////////////////////////////////////////////////////////////
	// main()
	////////////////////////////////////////////////////////////
	
	private static void usage() {
		System.err.println("java -cp hadoop-core.jar " + JobSimulator.class.getCanonicalName() + 
				" [-h] [-l logpath] [-m #maps/job] [-r #reduces/job] taskTrackerClusterSize");
	}

	/**
	 * Start the JobTracker process.  This is used only for debugging.  As a rule,
	 * JobTracker should be run as part of the DFS Namenode process.
	 */
	public static void main(String argv[])
	throws IOException, InterruptedException {
		java.lang.System.err.print("====================================== ");
		java.lang.System.err.print("Job Simulator");
		java.lang.System.err.println(" ======================================");
		
		if (argv.length < 1) {
			usage();
			System.exit(0);
		}

		String path = null;
		int jobs = 10;
		int maps = 100;
		int reduces = 100;
		boolean startJobTracker = false;
		int clusterSize = 0;
		for (int i = 0; i < argv.length; i++) {
			if (argv[i].startsWith("-j")) {
				startJobTracker = true;
			}
			else if (argv[i].startsWith("-l")) {
				path = argv[++i];
			}
			else if (argv[i].startsWith("-h")) {
				usage();
				System.exit(0);
			}
			else if (argv[i].startsWith("-j")) {
				jobs = Integer.parseInt(argv[++i]);
			}
			else if (argv[i].startsWith("-m")) {
				maps = Integer.parseInt(argv[++i]);
			}
			else if (argv[i].startsWith("-r")) {
				reduces = Integer.parseInt(argv[++i]);
			}
		}
		clusterSize = Integer.parseInt(argv[argv.length - 1]);

		try {
			if (startJobTracker) {
				JobTrackerImpl tracker = (JobTrackerImpl) JobTracker.startTracker(new JobConf());
				JobSimulator simulator = new JobSimulator(clusterSize, jobs, maps, reduces);
				simulator.start();
				tracker.offerService();
			}
			else {
				FileOutputStream utilityLog = null;
				FileOutputStream callLog = null;
				if (path != null) {
					callLog = new FileOutputStream(path + "/call.log");
					utilityLog = new FileOutputStream(path + "/utility.log");
				}
				JobSimulator simulator = new JobSimulator(clusterSize, jobs, maps, reduces, utilityLog, callLog);
				simulator.start();
				simulator.join();
			}
		} catch (Throwable e) {
			e.printStackTrace();
			java.lang.System.exit(-1);
		}
	}
}
