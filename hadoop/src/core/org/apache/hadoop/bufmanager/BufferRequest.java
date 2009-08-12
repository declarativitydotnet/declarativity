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
import java.util.ArrayList;
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
import org.apache.hadoop.mapred.IFile.Reader;
import org.apache.hadoop.mapred.IFile.Writer;
import org.apache.hadoop.mapred.Merger.Segment;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferRequest<K extends Object, V extends Object> implements Comparable<BufferRequest>, Writable {
	private class MapFile {
		public FSDataInputStream fsin;
		public long length;
		
		public MapFile(FSDataInputStream fsin, long length) {
			this.fsin = fsin;
			this.length = length;
		}
		
	}
	
	private TaskAttemptID taskid;
	
	private int partition;
	
	private String source = null;
	
	private InetSocketAddress sink = null;
	
	private IFile.Writer<K, V> writer = null;
	
	private List<MapFile> files;
	
	private boolean open = false;
	
	private FSDataOutputStream out = null;
	
	public BufferRequest() {
		this.files = new ArrayList<MapFile>();
	}
	
	public BufferRequest(TaskAttemptID taskid, int partition, String source, InetSocketAddress sink) {
		this.taskid    = taskid;
		this.partition = partition;
		this.source = source;
		this.sink   = sink;
		this.files = new ArrayList<MapFile>();
	}
	
	public String toString() {
		return "request buffer task " + taskid + " partition " + partition + " from " + source + " to " + sink;
	}
	
	public Integer partition() {
		return this.partition;
	}
	
	public void add(DataInputBuffer key, DataInputBuffer value) throws IOException {
		synchronized (this) {
			if (open) {
				this.writer.append(key, value);
			}
		}
	}
	
	public void open(JobConf conf, FileSystem localFs, int numSpills) throws IOException {
	    Class<K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
	    Class<V> valClass = (Class<V>)conf.getMapOutputValueClass();
	    
	    CompressionCodec codec = null;
		if (conf.getCompressMapOutput()) {
			Class<? extends CompressionCodec> codecClass =
				conf.getMapOutputCompressorClass(DefaultCodec.class);
			codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
		}
	    
		this.out = connect(-1);
		this.writer = new Writer<K, V>(conf, out, keyClass, valClass, codec);
		
		MapOutputFile mapOutputFile = new MapOutputFile();
		mapOutputFile.setConf(conf);
		for(int i = 0; i < numSpills; i++) {
			Path outputFile = mapOutputFile.getSpillFile(this.taskid, i);
			Path indexFile = mapOutputFile.getSpillIndexFile(this.taskid, i);
			
			FSDataInputStream indexIn = localFs.open(indexFile);
			indexIn.seek(this.partition * JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
			
			long segmentOffset = indexIn.readLong();
			long rawSegmentLength = indexIn.readLong();
			long segmentLength = indexIn.readLong();
			indexIn.close();
			FSDataInputStream in = localFs.open(outputFile);
			in.seek(segmentOffset);
			this.files.add(new MapFile(in, segmentLength));
		}

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
		long segmentLength    = indexIn.readLong();
		
		FSDataInputStream outIn = localFs.open(finalOutputFile);
		outIn.seek(segmentOffset);
		this.files.add(new MapFile(outIn, segmentLength));
		
		this.out = connect(segmentLength);
		
		this.open = true;
	}
	
	private FSDataOutputStream connect(long length) throws IOException {
		try {
			Socket socket = new Socket();
			socket.connect(this.sink);
			FSDataOutputStream out = new FSDataOutputStream(socket.getOutputStream());
			this.taskid.write(out);
			out.writeLong(length);
			return out;
		} catch (IOException e) {
			e.printStackTrace();
			throw e;
		}
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

	public void close() throws IOException {
		synchronized (this) {
			System.err.println("CLOSING BUFFER REQUEST " + this);
			open = false;
			flushBuffer();
			if (this.writer != null) {
				this.writer.close();
				this.out.close();
			}
			else {
				out.close();
			}
		}
	}
	
	public synchronized void flushBuffer() throws IOException {
		out.flush();
	}
	
	public synchronized void flushFile() throws IOException {
		byte [] output = new byte[1500];
		for (MapFile file : files) {
			long bytes = file.length;
			while (open && bytes > 0) {
				int bytesread = file.fsin.read(output, 0, Math.min((int) bytes, output.length));
				synchronized (this) {
					out.write(output, 0, bytesread);
				}
				bytes -= bytesread;
			}
		}
		this.files.clear();
	}

	@Override
	public int compareTo(BufferRequest o) {
		if (this.taskid.compareTo(o.taskid) != 0) {
			return this.taskid.compareTo(o.taskid);
		}
		else {
			return o.partition - this.partition;
		}
	}
}
