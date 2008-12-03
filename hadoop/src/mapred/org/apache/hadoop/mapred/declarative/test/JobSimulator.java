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
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Partitioner;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.RunningJob;
import org.apache.hadoop.mapred.SequenceFileInputFormat;
import org.apache.hadoop.mapred.lib.NullOutputFormat;
import org.apache.hadoop.util.Tool;

public class JobSimulator extends Thread {
	private class Job extends Configured 
	implements Runnable, Tool, 
			   Mapper<IntWritable, IntWritable, IntWritable, IntWritable>,  
			   Reducer<IntWritable, IntWritable, IntWritable, IntWritable>,  
			   Partitioner<IntWritable, IntWritable> {
		private String name;
		private int maps;
		private int reduces;
		private Path input;
		
		public Job(String name, int maps, int reduces, Path input) {
			this.name = name;
			this.maps = maps;
			this.reduces = reduces;
			this.input = input;
		}

		public void run() {
			try {
				JobConf job = new JobConf();
				job.setNumMapTasks(maps);
				job.setNumReduceTasks(reduces);
				job.setMapperClass(Job.class);
				job.setMapOutputKeyClass(IntWritable.class);
				job.setMapOutputValueClass(IntWritable.class);
				job.setReducerClass(Job.class);
				job.setOutputFormat(NullOutputFormat.class);
				job.setInputFormat(SequenceFileInputFormat.class);
				job.setSpeculativeExecution(false);
				job.setJobName(name);
				FileInputFormat.addInputPath(job, input);

				RunningJob runner = JobClient.runJob(job);
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
		}

		public void close() throws IOException {
		}

		public void reduce(IntWritable key, Iterator<IntWritable> values,
				OutputCollector<IntWritable, IntWritable> output,
				Reporter reporter) throws IOException {
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
		JobConf  conf = new JobConf();
		FileSystem fs = null;
		Path tempPath = new Path("/tmp/job.simulator.data");
		try {
			Random random = new Random();
			fs = FileSystem.get(conf);
			SequenceFile.Writer writer = 
				SequenceFile.createWriter(fs, conf, tempPath, 
						IntWritable.class, IntWritable.class);
			writer.close();
			while (!isInterrupted()) {
				try {
					sleep(20000);
					String name = "job" + jobs.size();
					Job job = new Job("job"+jobs.size(), 2, 2, tempPath);
					jobs.add(job);
					this.executor.execute(job);
				} catch (InterruptedException e) { }
			}
		} catch (Throwable t) {
			t.printStackTrace();
		}
		finally {
			if (fs != null) {
				try {
					fs.delete(tempPath, true);
				} catch (IOException e) { }
			}
		}
	}
}
