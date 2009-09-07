package org.apache.hadoop.examples;

import java.io.IOException;
import java.util.Iterator;
import java.util.Random;

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
import org.apache.hadoop.mapred.SequenceFileInputFormat;
import org.apache.hadoop.mapred.SequenceFileOutputFormat;
import org.apache.hadoop.mapred.lib.InverseMapper;
import org.apache.hadoop.mapred.lib.LongSumReducer;
import org.apache.hadoop.mapred.lib.RegexMapper;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

public class TopK extends Configured implements Tool {

	public class LongMapper<K> extends MapReduceBase
	implements Mapper<K, Text, Text, LongWritable> {

		public void configure(JobConf job) {
		}

		public void map(K key, Text value,
				OutputCollector<Text, LongWritable> output,
				Reporter reporter)
		throws IOException {
			String text = value.toString();
			output.collect(new Text(text), new LongWritable(1));
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
			while (values.hasNext() && k++ < count) {
				output.collect(key, values.next());
			}
		}
	}

	private TopK() {}                               // singleton

	public int run(String[] args) throws Exception {
		if (args.length < 3) {
			System.out.println("TopK <inDir> <outDir> <K> [<regexpr> [<group>]]");
			ToolRunner.printGenericCommandUsage(System.out);
			return -1;
		}

		Path tempDir =
			new Path("topk-temp-"+
					Integer.toString(new Random().nextInt(Integer.MAX_VALUE)));

		JobConf topkJob = new JobConf(getConf(), TopK.class);

		try {

			topkJob.setJobName("topk-search");

			FileInputFormat.setInputPaths(topkJob, args[0]);

			if (args.length == 4) {
				topkJob.setMapperClass(RegexMapper.class);
				topkJob.set("mapred.mapper.regex", args[3]);
				if (args.length == 5)
					topkJob.set("mapred.mapper.regex.group", args[4]);
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

			JobClient.runJob(topkJob);

			JobConf sortJob = new JobConf(TopK.class);
			sortJob.setJobName("topk-sort");

			FileInputFormat.setInputPaths(sortJob, tempDir);
			sortJob.setInputFormat(SequenceFileInputFormat.class);

			sortJob.setMapperClass(InverseMapper.class);
			sortJob.setReducerClass(TopKReduce.class);
			sortJob.setInt("mapred.mapper.regex", Integer.parseInt(args[2]));

			sortJob.setNumReduceTasks(1);                 // write a single file
			FileOutputFormat.setOutputPath(sortJob, new Path(args[1]));
			sortJob.setOutputKeyComparatorClass           // sort by decreasing freq
			(LongWritable.DecreasingComparator.class);

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