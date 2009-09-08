package org.apache.hadoop.examples;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
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

	public static class LongMapper<K> extends MapReduceBase
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

	/** A {@link Mapper} that extracts text matching a regular expression. */
	public static class TopKRegexMapper<K> extends MapReduceBase
	implements Mapper<K, Text, Text, LongWritable> {

		private Pattern pattern;
		private int group;

		private final static LongWritable one = new LongWritable(1);
		private Text word = new Text();

		public void configure(JobConf job) {
			pattern = Pattern.compile(job.get("mapred.mapper.regex"));
			group = job.getInt("mapred.mapper.regex.group", 0);
		}

		public void map(K key, Text value,
				OutputCollector<Text, LongWritable> output,
				Reporter reporter) throws IOException {
			String line = value.toString();
			StringTokenizer itr = new StringTokenizer(line);

			while (itr.hasMoreTokens()) {
				String next = itr.nextToken();
				Matcher matcher = pattern.matcher(next);
				if (matcher.find()) {
					word.set(next);
					output.collect(word, one);
				}
			}
		}
	}

	/**
	 * A reducer class that just emits the sum of the input values.
	 */
	public static class TopKReduce extends MapReduceBase
	implements Reducer<LongWritable, Text, LongWritable, Text> {

		int k = 0;
		int count = 0;

		public void configure(JobConf job) {
			k = job.getInt("mapred.reduce.topk.k", 1);
			count = 0;
		}

		public void reduce(LongWritable key, Iterator<Text> values,
				OutputCollector<LongWritable, Text> output, 
				Reporter reporter) throws IOException {
			while (values.hasNext() && count++ < k) {
				output.collect(key, values.next());
			}
		}
	}

	private TopK() {}                               // singleton
	
	private void printUsage() {
		System.out.println("TopK [-m mappers] [-r reducers] <inDir> <outDir> <K> [<regexpr> [<group>]]");
		ToolRunner.printGenericCommandUsage(System.out);
	}

	public int run(String[] args) throws Exception {
		if (args.length < 3) {
			printUsage();
			return -1;
		}
		
		JobClient client = new JobClient();

		Path tempDir =
			new Path("topk-temp-"+
					Integer.toString(new Random().nextInt(Integer.MAX_VALUE)));

		JobConf topkJob = new JobConf(getConf(), TopK.class);

		try {

			topkJob.setJobName("topk-search");

		    List<String> other_args = new ArrayList<String>();
		    for(int i=0; i < args.length; ++i) {
		      try {
		        if ("-m".equals(args[i])) {
		          topkJob.setNumMapTasks(Integer.parseInt(args[++i]));
		        } else if ("-r".equals(args[i])) {
		          topkJob.setNumReduceTasks(Integer.parseInt(args[++i]));
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
		    
			FileInputFormat.setInputPaths(topkJob, other_args.get(0));

			if (other_args.size() >= 4) {
				topkJob.setMapperClass(TopKRegexMapper.class);
				topkJob.set("mapred.mapper.regex", other_args.get(3));
				if (args.length == 5)
					topkJob.set("mapred.mapper.regex.group", other_args.get(4));
			}
			else {
				topkJob.setMapperClass(LongMapper.class);
			}

			topkJob.setCombinerClass(LongSumReducer.class);
			topkJob.setReducerClass(LongSumReducer.class);

			FileOutputFormat.setOutputPath(topkJob, tempDir);
			topkJob.setOutputFormat(SequenceFileOutputFormat.class);
			topkJob.setOutputKeyClass(Text.class);
			topkJob.setOutputValueClass(LongWritable.class);

			RunningJob topkHandle = client.submitJob(topkJob);

			JobConf sortJob = new JobConf(TopK.class);
			sortJob.setJobName("topk-sort");

			FileInputFormat.setInputPaths(sortJob, tempDir);
			sortJob.setInputFormat(SequenceFileInputFormat.class);

			sortJob.setMapperClass(InverseMapper.class);
			// sortJob.setCombinerClass(TopKReduce.class);
			sortJob.setReducerClass(TopKReduce.class);
			sortJob.setInt("mapred.reduce.topk.k", Integer.parseInt(other_args.get(2)));

			sortJob.setNumReduceTasks(1);                 // write a single file
			FileOutputFormat.setOutputPath(sortJob, new Path(other_args.get(1)));
			sortJob.setOutputKeyComparatorClass           // sort by decreasing freq
			(LongWritable.DecreasingComparator.class);
			
			sortJob.set("mapred.job.pipeline", topkHandle.getJobID());

			JobClient.runJob(sortJob);
		}
		finally {
			FileSystem.get(topkJob).delete(tempDir, true);
		}
		return 0;
	}

	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new Configuration(), new TopK(), args);
		System.exit(res);
	}

}