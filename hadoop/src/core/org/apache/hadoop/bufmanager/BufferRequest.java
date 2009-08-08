package org.apache.hadoop.bufmanager;

import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.Compressor;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapOutputFile;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.IFile.Writer;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferRequest<K extends Object, V extends Object> implements Writable, Runnable {
	private TaskAttemptID taskid;
	
	private int partition;
	
	private String source = null;
	
	private InetSocketAddress sink = null;
	
	private IFile.Writer<K, V> writer = null;
	
	private FSDataInputStream fsin = null;
	
	private long segmentLength = 0;
	
	private boolean open = false;
	
	public BufferRequest() {}
	
	public BufferRequest(TaskAttemptID taskid, int partition, String source, InetSocketAddress sink) {
		this.taskid    = taskid;
		this.partition = partition;
		this.source = source;
		this.sink   = sink;
	}
	
	public String toString() {
		return "request buffer task " + taskid + " partition " + partition + " from " + source + " to " + sink;
	}
	
	public IFile.Writer<K, V> writer() {
		return this.writer;
	}
	
	public void open(JobConf conf) throws IOException {
	    Class<K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
	    Class<V> valClass = (Class<V>)conf.getMapOutputValueClass();
	    
	    CompressionCodec codec = null;
		if (conf.getCompressMapOutput()) {
			Class<? extends CompressionCodec> codecClass =
				conf.getMapOutputCompressorClass(DefaultCodec.class);
			codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
		}
	    
		Socket socket = new Socket();
		socket.connect(this.sink);

		FSDataOutputStream out = new FSDataOutputStream(socket.getOutputStream());
		this.taskid.write(out);
		out.writeLong(-1);
		
		this.writer = new Writer<K, V>(conf, out, keyClass, valClass, codec);

		this.open = true;
	}
	
	public void open(Configuration conf, FileSystem localFs) throws IOException {
		MapOutputFile mapOutputFile = new MapOutputFile();
		mapOutputFile.setConf(conf);
		
		Path finalOutputFile = mapOutputFile.getOutputFile(this.taskid);
		Path finalIndexFile = mapOutputFile.getOutputIndexFile(this.taskid);
		
		FSDataInputStream indexIn = localFs.open(finalIndexFile);
		indexIn.seek(this.partition * JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
		long segmentOffset    = indexIn.readLong();
		long rawSegmentLength = indexIn.readLong();
		this.segmentLength    = indexIn.readLong();
		
		this.fsin = localFs.open(finalOutputFile);
		this.fsin.seek(segmentOffset);
		
		this.open = true;
	}
	
	public TaskAttemptID taskid() {
		return this.taskid;
	}
	
	public String source() {
		return this.source;
	}
	
	public InetSocketAddress sink() {
		return this.sink;
	}
	
	@Override
	public void readFields(DataInput in) throws IOException {
		this.taskid = new TaskAttemptID();
		this.taskid.readFields(in);
		
		this.source = WritableUtils.readString(in);
		
		String sinkHost = WritableUtils.readString(in);
		int    sinkPort = WritableUtils.readVInt(in);
		this.sink = new InetSocketAddress(sinkHost, sinkPort);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		if (this.source == null || this.sink == null)
			throw new IOException("No source/sink in request!");
		
		this.taskid.write(out);
		WritableUtils.writeString(out, this.source);
		
		WritableUtils.writeString(out, this.sink.getHostName());
		WritableUtils.writeVInt(out, this.sink.getPort());
	}

	public void cancel() throws IOException {
		this.open = false;
	}
	
	@Override
	public void run() {
		
		DataOutputStream out = null;
		try {
			Socket socket = new Socket();
			socket.connect(this.sink);

			out = new DataOutputStream(socket.getOutputStream());
			this.taskid.write(out);
			out.writeLong(this.segmentLength);

			byte [] output = new byte[256];
			long bytes = this.segmentLength;
			while (open && bytes > 0) {
				int bytesread = this.fsin.read(output, 0, (bytes < output.length ? (int) bytes : output.length));
				out.write(output, 0, bytesread);
				bytes -= bytesread;
			}
		} catch (IOException e) { e.printStackTrace(); }
		finally {
			try {
				out.flush();
				out.close();
			} catch (IOException e) { e.printStackTrace(); }
		}
	}
}
