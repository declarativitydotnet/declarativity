/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.hadoop.examples;



import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.HashMap;
import java.util.StringTokenizer;
import java.util.regex.Pattern;

import java.net.InetAddress;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.ArrayWritable;
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
import org.apache.hadoop.mapred.SkipBadRecords;


import org.apache.hadoop.mapred.bufmanager.JBuffer;

//import bfs.telemetry.SystemStats;
//import bfs.telemetry.SystemStats.SystemStatEntry;

/**
 * This is an example Hadoop Map/Reduce application.
 * It reads the text input files, breaks each line into words
 * and counts them. The output is a locally sorted list of words and the 
 * count of how often they occurred.
 *
 * To run: bin/hadoop jar build/hadoop-examples.jar wordcount
 *            [-s] [-m <i>maps</i>] [-r <i>reduces</i>] <i>in-dir</i> <i>out-dir</i> 
 */
public class CQ extends Configured implements Tool {

        /**
         * Counts the words in each line.
         * For each line of input, break the line into words and emit them as
         * (<b>word</b>, <b>1</b>).
         */
        public static enum SystemStatEntry {
                USER(0, "normal processes executing in user mode"),
                        NICE(1, "niced processes executing in user mode"),
                        SYSTEM(2, "processes executing in kernel mode"),
                        IDLE(3, "twiddling thumbs"),
                        IOWAIT(4, "waiting for I/O to complete"),
                        IRQ(5, "servicing interrupts"),
                        SOFTIRQ(6, "softirq: servicing softirqs"),
                        STEAL(7, "involuntary wait (usually host virtual machine) time"),
                        GUEST(8, "guest virtual machine time"),
                        LOAD_1(9, "1 minute load average"); /* ,
                                                               LOAD_5(10, "5 minute load average"),
                                                               LOAD_15(11, "15 minute load average");
                                                             */
                public final int offset;
                public final String name;
                private SystemStatEntry(int o, String n) {
                        this.offset = o; this.name = n;
                }
        }

        public static class SystemStats {

       
                public int getInt(SystemStatEntry e) {
                        return cols[e.offset].intValue();
                }
                public String getStrByOffset(int o) {
                        return cols[o].toString();
                }

                public float getFloat(SystemStatEntry e) {
                        return cols[e.offset].floatValue();
                }

                final Pattern whiteSpace = Pattern.compile("\\s+");
                Number cols[];
                public SystemStats() throws IOException {
                        File f = new File("/proc/stat");
                        BufferedReader in = new BufferedReader(new FileReader(f));
                        String info = in.readLine();
                        if(!info.startsWith("cpu  ")) { throw new IllegalStateException("Can't parse /proc/stat!"); }
                        info = info.substring(5);
                        String[] tok = whiteSpace.split(info);
                        cols = new Number[tok.length + 3];
                        for(int i = 0; i < tok.length; i++) {
                                try {
                                        cols[i] = Long.parseLong(tok[i]);
                                } catch(Exception e) {
                                        // deal w/ index out of bound, number format, etc by ignoring them
                                }
                        }
                        in.close();
                        f = new File("/proc/loadavg");
                        in = new BufferedReader(new FileReader(f));
                        info = in.readLine();
                        tok = whiteSpace.split(info);
                        cols[cols.length-3] = Double.parseDouble(tok[0]);
                        cols[cols.length-2] = Double.parseDouble(tok[1]);
                        cols[cols.length-1] = Double.parseDouble(tok[2]);
                }
                boolean verbose = true;
                @Override
                        public String toString() {
                                StringBuilder sb = new StringBuilder();
                                sb.append("<" + super.toString() + ": ");
                                for(SystemStatEntry e: SystemStatEntry.values()) {
                                        System.err.println("offset: "+ e.offset);
                                        System.err.println("val: "+cols[e.offset]);
                                        sb.append("\n\t(" + e + ", " + cols[e.offset] + ( verbose ? ", " + e.name  : "" )+ ")");
                                }
                                sb.append(">");
                                return sb.toString();
                        }

                public int totalJiffies() {
                        int sum = 0;
                        for (SystemStatEntry e: SystemStatEntry.values()) {
                                if (e.offset < SystemStatEntry.LOAD_1.offset) {
                                        sum += cols[e.offset].intValue();
                                }
                        }
                        return sum;
                }

        }	


        public static class MapClass extends MapReduceBase
                implements Mapper<LongWritable, Text, Text, Text> {

                        private final static IntWritable one = new IntWritable(1);
                        private Text word = new Text();
                        public void sleep(int s) {
                          try {
                            Thread.sleep(s);
                          } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                          }
                        }
                        public void map(LongWritable key, Text value, 
                                        OutputCollector<Text, Text> output, 
                                        Reporter reporter) throws IOException {
                                java.net.InetAddress localMachine = java.net.InetAddress.getLocalHost();
                                String hn = localMachine.getHostName();

                                float su = 0;
                                float ss = 0;
                                float st = 0;
                                while (true) {
                                        SystemStats stat = new SystemStats();
                                        word.set(hn);
                                        // I was having a lot of trouble with ArrayWritable...
                                        StringBuilder sb = new StringBuilder();
                                        for (int i=0; i < 10; i++) {
                                          sb.append(stat.getStrByOffset(i));
                                          sb.append(",");
                                        }
                                        // total jiffies go at the end
                                        sb.append(stat.totalJiffies());
                                        Text statList = new Text(sb.toString());
                                        output.collect(word, statList);

                                        // try to force the buffer
                                        JBuffer jb = (JBuffer) output;
                                        while (!jb.force()) {
                                                System.err.println("Can't force yet....");
                                                reporter.progress();
                                                sleep(1000);
                                        }
                                        // calc load here.
                                        float u = stat.getFloat(SystemStatEntry.USER);
                                        float s = stat.getFloat(SystemStatEntry.SYSTEM);
                                        float j = stat.totalJiffies();
                                        float l = ((u-su) + (s-su)) / (j-st);

                                        System.err.println("M load: " + l );
                                        System.err.println("ie, "+u+","+su+" : "+j+","+st);
                                        su = u; ss = s; st = j;
            
                                        sleep(5000);
                                }


                        }
                }

        /**
         * A reducer class that just emits the sum of the input values.
         */
        public static class Reduce extends MapReduceBase 
                implements Reducer<Text, Text, Text, IntWritable> {

                static float suser = 0;
                static float ssystem = 0;
                static float stotal = 0;
                static HashMap state = new HashMap();
                        public void reduce(Text key, Iterator<Text> values,
                                        OutputCollector<Text, IntWritable> output, 
                                        Reporter reporter) throws IOException {
                                int sum = 0;
                                while (values.hasNext()) {
                                        Text v = values.next();
                                        System.err.println("OK, got output: " + v.toString());
                                        String[] items = v.toString().split(",");
                                        float user = Float.parseFloat(items[0]);
                                        float system = Float.parseFloat(items[2]);
                                        float total = Float.parseFloat(items[10]);

                                        float load = ((user - suser) + (system - ssystem)) / (total - stotal);
                                        System.err.println("load: " + load);
                                        System.err.println("ie, (" + (user-suser) + " + " + (system-ssystem) + ") / "+(total-stotal));

                                        System.err.println("user state change: " + suser + " --> " + user);
                                        suser = user;
                                        ssystem = system;
                                        stotal = total;
                                }
                        }
                }

        static int printUsage() {
                System.out.println("cq [-w <window>] [-m <maps>] [-r <reduces>] <input> <output>");
                ToolRunner.printGenericCommandUsage(System.out);
                return -1;
        }

        /**
         * The main driver for word count map/reduce program.
         * Invoke this method to submit the map/reduce job.
         * @throws IOException When there is communication problems with the 
         *                     job tracker.
         */
        public int run(String[] args) throws Exception {
                JobConf conf = new JobConf(getConf(), CQ.class);
                conf.setJobName("cq");

                // the keys are words (strings)
                conf.setOutputKeyClass(Text.class);
                // the values are counts (ints)
                conf.setOutputValueClass(Text.class);

                conf.setMapperClass(MapClass.class);        
                /* DO NOT USE A COMBINER
                   conf.setCombinerClass(Reduce.class);
                 */
                conf.setReducerClass(Reduce.class);

                List<String> other_args = new ArrayList<String>();
                for(int i=0; i < args.length; ++i) {
                        try {
                                if ("-w".equals(args[i])) {
                                        conf.setInt("mapred.reduce.window", Integer.parseInt(args[++i]));
                                        conf.setBoolean("mapred.map.pipeline", false);
                                } else if ("-m".equals(args[i])) {
                                        conf.setNumMapTasks(Integer.parseInt(args[++i]));
                                } else if ("-r".equals(args[i])) {
                                        conf.setNumReduceTasks(Integer.parseInt(args[++i]));
                                } else {
                                        other_args.add(args[i]);
                                }
                        } catch (NumberFormatException except) {
                                System.out.println("ERROR: Integer expected instead of " + args[i]);
                                return printUsage();
                        } catch (ArrayIndexOutOfBoundsException except) {
                                System.out.println("ERROR: Required parameter missing from " +
                                                args[i-1]);
                                return printUsage();
                        }
                }
                // Make sure there are exactly 2 parameters left.
                if (other_args.size() != 2) {
                        System.out.println("ERROR: Wrong number of parameters: " +
                                        other_args.size() + " instead of 2.");
                        return printUsage();
                }
                FileInputFormat.setInputPaths(conf, other_args.get(0));
                FileOutputFormat.setOutputPath(conf, new Path(other_args.get(1)));

                JobClient.runJob(conf);
                return 0;
        }


        public static void main(String[] args) throws Exception {
                int res = ToolRunner.run(new Configuration(), new CQ(), args);
                System.exit(res);
        }

}
