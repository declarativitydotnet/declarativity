package org.apache.hadoop.examples;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.TreeSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.RunningJob;
import org.apache.hadoop.mapred.SequenceFileInputFormat;
import org.apache.hadoop.mapred.SequenceFileOutputFormat;
import org.apache.hadoop.mapred.lib.InverseMapper;
import org.apache.hadoop.mapred.lib.LongSumReducer;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

public class TopK extends Configured implements Tool {

	public static class WordcountMapper<K> extends MapReduceBase
	implements Mapper<K, Text, Text, LongWritable> {

	    private final static LongWritable one = new LongWritable(1);
	    private Text word = new Text();

		public void configure(JobConf job) {
		}

		public void map(K key, Text value,
				OutputCollector<Text, LongWritable> output,
				Reporter reporter)
		throws IOException {
			String line = value.toString();
		    StringTokenizer itr = new StringTokenizer(line);

		    while (itr.hasMoreTokens()) {
		    	word.set(itr.nextToken());
		    	output.collect(word, one);
		    }
		}
	}
	
	public static class WikiMapper<K> extends MapReduceBase
	implements Mapper<K, Text, Text, LongWritable> {

	    private final static LongWritable one = new LongWritable(1);
	    private Text word = new Text();

		public void configure(JobConf job) {
		}

		public void map(K key, Text value,
				OutputCollector<Text, LongWritable> output,
				Reporter reporter) throws IOException {
			String line = value.toString();
			int start = line.indexOf("</articles>");
			if (start >= 0) {
				line = line.substring(start + "</articles>".length());
				StringTokenizer itr = new StringTokenizer(line);

				while (itr.hasMoreTokens()) {
					word.set(itr.nextToken());
					output.collect(word, one);
				}
			}
		}
	}
	
	/**
	 * A reducer class that just emits the sum of the input values.
	 */
	public static class WordcountReduceFilter extends MapReduceBase implements
			Reducer<Text, LongWritable, Text, LongWritable> {

		private static class TopKRecord implements Comparable<TopKRecord> {
			public Text key;
			public long sum;

			public TopKRecord(Text key, long sum) {
				this.key = key;
				this.sum = sum;
			}

			@Override
			public boolean equals(Object obj) {
				if (!(obj instanceof TopKRecord))
					return false;

				TopKRecord other = (TopKRecord) obj;
				if (other.sum == this.sum && other.key.equals(this.key))
					return true;
				else
					return false;
			}

			@Override
			public int hashCode() {
				return this.key.hashCode() ^ (int) this.sum;
			}

			@Override
			public int compareTo(TopKRecord other) {
				if (this.sum == other.sum)
					return this.key.compareTo(other.key);

				return (this.sum < other.sum ? -1 : 1);
			}
		}

		private final TreeSet<TopKRecord> heap = new TreeSet<TopKRecord>();
		private OutputCollector<Text, LongWritable> target = null;
		
		int k = 0;

		@Override
		public void configure(JobConf job) {
			k = job.getInt("mapred.reduce.topk.k", 1);
		}

		public void reduce(Text key, Iterator<LongWritable> values,
				OutputCollector<Text, LongWritable> output, Reporter reporter)
				throws IOException {
			/* On first call, remember the output destination (XXX hack) */
			if (target == null)
				target = output;

			int sum = 0;
			while (values.hasNext()) {
				sum += values.next().get();
			}

			// NB: We need to copy the key, because it is overwritten by caller
			this.heap.add(new TopKRecord(new Text(key), sum));
			if (this.heap.size() >= k) {
				TopKRecord removed = this.heap.pollFirst();
				if (removed == null)
					throw new IllegalStateException();
			}
		}

		@Override
		public void close() throws IOException {
			if (this.target == null) {
				assert(this.heap.size() == 0);
				return;
			}

			for (TopKRecord rec : this.heap) {
				this.target.collect(rec.key, new LongWritable(rec.sum));
			}

			this.heap.clear();
			this.target = null;
		}
	}

	/**
	 * A reducer class that just emits the sum of the input values.
	 */
	public static class TopKReduce extends MapReduceBase
	implements Reducer<LongWritable, Text, LongWritable, Text> {

		int k = 0;
		int count = 0;
		static List<List<String>> topklist = new ArrayList<List<String>>();
		boolean compare = false;

		public void configure(JobConf job) {
			k = job.getInt("mapred.reduce.topk.k", 1);
			count = 0;
			compare = job.getBoolean("mapred.job.comparelists", false);
			if (compare) topklist.add(new ArrayList<String>());
		}
		
		public void reduce(LongWritable key, Iterator<Text> values,
				OutputCollector<LongWritable, Text> output,
				Reporter reporter) throws IOException {
			
			List<String> topk = compare ? topklist.get(topklist.size() - 1) : null;
			while (values.hasNext() && count++ < k) {
				Text value = values.next();
				if (topk != null) topk.add(value.toString());
				output.collect(key, value);
			}
		}
		
		@Override
		public void close() throws IOException {
			if (!compare || topklist.size() <= 1) return;
			
			List<String> topk = topklist.get(topklist.size() - 1);
			System.err.println("TOPK: compare most recent topk " + topklist.size() + " to previous topks.");
			for (int j = topklist.size() - 2; j >= 0; j--) {
				List<String> previous = topklist.get(j);
				if (previous != null) {
					for (int i = 0; i < previous.size(); i++) {
						if (!previous.get(i).equals(topk.get(i))) {
							System.err.println("\tTOPK: list " + j + " matched " + 
									           i + " values in current.");
							break;
						}
					}
				}
			}
			System.err.println("TOPK: end compare");
		}
	}

	private TopK() {}                               // singleton

	private void printUsage() {
		System.out.println("TopK [-s frequency] [-xcpPR] [-m mappers] [-r reducers] <inDir> <outDir> <K>");
		System.out.println("\t-p intra-job pipelining\n\t-P inter-job pipelining\n\t-R do not reduce job 1 output\n\t-x use xml article mapper\n\t-c compare topk lists");
		ToolRunner.printGenericCommandUsage(System.out);
	}

	public int run(String[] args) throws Exception {
		if (args.length < 3) {
			printUsage();
			return -1;
		}

		Path tempDir =
			new Path("topk-temp-"+
					Integer.toString(new Random().nextInt(Integer.MAX_VALUE)));

		try {
			boolean pipelineJob  = false;
			boolean reduceOutput = true;

			JobConf wordcountJob = new JobConf(getConf(), TopK.class);
			wordcountJob.setJobName("topk-wordcount");

			JobConf topkJob = new JobConf(getConf(), TopK.class);
			topkJob.setJobName("topk-select");

			boolean xmlmapper = false;
		    List<String> other_args = new ArrayList<String>();
		    for(int i=0; i < args.length; ++i) {
		      try {
		          if ("-s".equals(args[i])) {
		        	float freq = Float.parseFloat(args[++i]);
		        	/* Jobs will perform snapshots */
		          	wordcountJob.setFloat("mapred.snapshot.frequency", freq);
		          	topkJob.setBoolean("mapred.job.input.snapshots", true);

		          	/* Wordcount will pipeline. */
		          	wordcountJob.setBoolean("mapred.map.pipeline", true);
		          	wordcountJob.setBoolean("mapred.reduce.pipeline", true);
		          	/* TopK does not pipeline. */
		          	topkJob.setBoolean("mapred.map.pipeline", true);
		          	topkJob.setBoolean("mapred.reduce.pipeline", false);
		        	pipelineJob = true;
		          } else if ("-R".equals(args[i])) {
		        	  reduceOutput = false;
		          } else if ("-c".equals(args[i])) {
		        	  topkJob.setBoolean("mapred.job.comparelists", true);
		          } else if ("-x".equals(args[i])) {
		        	  xmlmapper = true;
		          } else if ("-p".equals(args[i])) {
		          	wordcountJob.setBoolean("mapred.map.pipeline", true);
		          	topkJob.setBoolean("mapred.map.pipeline", true);
		          } else if ("-P".equals(args[i])) {
		    		pipelineJob = true;
		          	wordcountJob.setBoolean("mapred.reduce.pipeline", true);
		    	  } else if ("-m".equals(args[i])) {
		    		  wordcountJob.setNumMapTasks(Integer.parseInt(args[++i]));
		    	  } else if ("-r".equals(args[i])) {
		    		  wordcountJob.setNumReduceTasks(Integer.parseInt(args[++i]));
		    	  } else {
		    		  other_args.add(args[i]);
		    	  }
		      } catch (NumberFormatException except) {
		        System.out.println("ERROR: Integer expected instead of " + args[i]);
		        printUsage();
		        return -1;
		      } catch (ArrayIndexOutOfBoundsException except) {
		        System.out.println("ERROR: Required parameter missing from " +
		                           args[i-1]);
		        printUsage();
		        return -1;
		      }
		    }
		    // Make sure there are exactly 2 parameters left.
		    if (other_args.size() < 3) {
		      System.out.println("ERROR: Wrong number of parameters: " +
		                         other_args.size() + ".");
		      printUsage(); return -1;
		    }

			FileInputFormat.setInputPaths(wordcountJob, other_args.get(0));

			if (xmlmapper) {
				wordcountJob.setMapperClass(WikiMapper.class);
			}
			else {
				wordcountJob.setMapperClass(WordcountMapper.class);
			}
			wordcountJob.setCombinerClass(LongSumReducer.class);
			
			if (reduceOutput) {
				wordcountJob.setReducerClass(WordcountReduceFilter.class);
			}
			else {
				wordcountJob.setReducerClass(LongSumReducer.class);
			}

			FileOutputFormat.setOutputPath(wordcountJob, tempDir);
			wordcountJob.setOutputFormat(SequenceFileOutputFormat.class);
			wordcountJob.setOutputKeyClass(Text.class);
			wordcountJob.setOutputValueClass(LongWritable.class);
			wordcountJob.setInt("mapred.reduce.topk.k", Integer.parseInt(other_args.get(2)));


			FileInputFormat.setInputPaths(topkJob, tempDir);
			topkJob.setInputFormat(SequenceFileInputFormat.class);

			topkJob.setMapperClass(InverseMapper.class);
			topkJob.setCombinerClass(TopKReduce.class);
			topkJob.setReducerClass(TopKReduce.class);
			topkJob.setInt("mapred.reduce.topk.k", Integer.parseInt(other_args.get(2)));

			topkJob.setNumReduceTasks(1);                 // write a single file
			FileOutputFormat.setOutputPath(topkJob, new Path(other_args.get(1)));
			topkJob.setOutputKeyComparatorClass           // sort by decreasing freq
			(LongWritable.DecreasingComparator.class);

			if (pipelineJob) {
				JobClient  client  = new JobClient(wordcountJob);
				List<JobConf> jobs = new ArrayList<JobConf>();
				jobs.add(wordcountJob);
				jobs.add(topkJob);
				List<RunningJob> rjobs = client.submitJobs(jobs);
				for (int i = 0; i < rjobs.size(); i++) {
					RunningJob rjob = rjobs.get(i);
					JobConf job = jobs.get(i);
					client.report(rjob, job);
				}
			}
			else {
				JobClient.runJob(wordcountJob);
				JobClient.runJob(topkJob);
			}

			FileSystem.get(wordcountJob).delete(tempDir, true);
		}
		finally {
		}
		return 0;
	}

	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new Configuration(), new TopK(), args);
		System.exit(res);
	}

}