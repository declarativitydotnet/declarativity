package org.apache.hadoop.examples;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;
import java.util.TreeSet;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
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
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

public class TopKWordCount extends Configured implements Tool {

	/**
	 * Counts the words in each line. For each line of input, break the line
	 * into words and emit them as (<b>word</b>, <b>1</b>).
	 */
	public static class MapClass extends MapReduceBase implements
			Mapper<LongWritable, Text, Text, IntWritable> {

		private final static IntWritable one = new IntWritable(1);
		private Text word = new Text();

		public void map(LongWritable key, Text value,
				OutputCollector<Text, IntWritable> output, Reporter reporter)
				throws IOException {
			String line = value.toString();
			StringTokenizer itr = new StringTokenizer(line);
			while (itr.hasMoreTokens()) {
				word.set(itr.nextToken());
				output.collect(word, one);
			}
		}
	}

	/**
	 * A reducer class that just emits the sum of the input values.
	 */
	public static class Reduce extends MapReduceBase implements
			Reducer<Text, IntWritable, Text, IntWritable> {

		private static class TopKRecord implements Comparable<TopKRecord> {
			public Text key;
			public int sum;

			public TopKRecord(Text key, int sum) {
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
				return this.key.hashCode() ^ this.sum;
			}

			@Override
			public int compareTo(TopKRecord other) {
				if (this.sum == other.sum)
					return this.key.compareTo(other.key);

				return (this.sum < other.sum ? -1 : 1);
			}
		}

		private final TreeSet<TopKRecord> heap = new TreeSet<TopKRecord>();
		private OutputCollector<Text, IntWritable> target = null;
		
		int k = 0;

		@Override
		public void configure(JobConf job) {
			k = job.getInt("mapred.reduce.topk.k", 1);
		}

		public void reduce(Text key, Iterator<IntWritable> values,
				OutputCollector<Text, IntWritable> output, Reporter reporter)
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
				this.target.collect(rec.key, new IntWritable(rec.sum));
			}

			this.heap.clear();
			this.target = null;
		}
	}

	static int printUsage() {
		System.out.println("topkwordcount [-s <interval>] [-p] [-m <maps>] [-r <reduces>] <input> <output> K");
		ToolRunner.printGenericCommandUsage(System.out);
		return -1;
	}

	/**
	 * The main driver for word count map/reduce program. Invoke this method to
	 * submit the map/reduce job.
	 *
	 * @throws IOException
	 *             When there is communication problems with the job tracker.
	 */
	public int run(String[] args) throws Exception {
		JobConf conf = new JobConf(getConf(), TopKWordCount.class);
		conf.setJobName("topkwordcount");

		// the keys are words (strings)
		conf.setOutputKeyClass(Text.class);
		// the values are counts (ints)
		conf.setOutputValueClass(IntWritable.class);

		conf.setMapperClass(MapClass.class);
		conf.setCombinerClass(Reduce.class);
		conf.setReducerClass(Reduce.class);

		List<String> other_args = new ArrayList<String>();
		for (int i = 0; i < args.length; ++i) {
			try {
				if ("-s".equals(args[i])) {
		        	conf.setInt("mapred.snapshot.interval", Integer.parseInt(args[++i]));
		        	conf.setBoolean("mapred.map.pipeline", true);
		        	conf.setCombinerClass(null);
				} else if ("-p".equals(args[i])) {
					conf.setBoolean("mapred.map.pipeline", true);
		        	conf.setCombinerClass(null);
				} else if ("-m".equals(args[i])) {
					conf.setNumMapTasks(Integer.parseInt(args[++i]));
				} else if ("-r".equals(args[i])) {
					conf.setNumReduceTasks(Integer.parseInt(args[++i]));
				} else {
					other_args.add(args[i]);
				}
			} catch (NumberFormatException except) {
				System.out.println("ERROR: Integer expected instead of "
						+ args[i]);
				return printUsage();
			} catch (ArrayIndexOutOfBoundsException except) {
				System.out.println("ERROR: Required parameter missing from "
						+ args[i - 1]);
				return printUsage();
			}
		}
		// Make sure there are exactly 2 parameters left.
		if (other_args.size() != 3) {
			System.out.println("ERROR: Wrong number of parameters: "
					+ other_args.size() + " instead of 3.");
			return printUsage();
		}
		FileInputFormat.setInputPaths(conf, other_args.get(0));
		FileOutputFormat.setOutputPath(conf, new Path(other_args.get(1)));
		conf.setInt("mapred.reduce.topk.k", Integer.parseInt(other_args.get(2)));


		JobClient.runJob(conf);
		return 0;
	}

	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new Configuration(), new TopKWordCount(), args);
		System.exit(res);
	}
}
