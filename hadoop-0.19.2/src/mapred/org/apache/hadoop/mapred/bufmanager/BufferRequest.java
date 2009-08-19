package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
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
import org.apache.hadoop.mapred.IFileInputStream;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapOutputFile;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.IFile.Reader;
import org.apache.hadoop.mapred.IFile.Writer;
import org.apache.hadoop.mapred.Merger.Segment;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferRequest<K extends Object, V extends Object> implements Comparable<BufferRequest>, Writable, Runnable {
	
	public boolean delivered = false;
	
	private TaskAttemptID taskid;
	
	private int partition;
	
	private String source = null;
	
	private InetSocketAddress sink = null;
	
	private boolean open = false;
	
	private FSDataOutputStream out = null;
	
	private Configuration conf = null;
	
	private FileSystem localFS = null;
	
	private byte[] output = null;
	
	private boolean busy = false;
	
	private int flushPoint = 0;
	
	public BufferRequest() {
	}
	
	public BufferRequest(TaskAttemptID taskid, int partition, String source, InetSocketAddress sink) {
		this.taskid    = taskid;
		this.partition = partition;
		this.source = source;
		this.sink   = sink;
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
	
	@Override
	public String toString() {
		return "request buffer task " + taskid + " partition " + partition + " from " + source + " to " + sink;
	}
	
	public Integer partition() {
		return this.partition;
	}
	
	public boolean busy() {
		return this.busy;
	}
	
	public int flushPoint() {
		return this.flushPoint;
	}
	
	public void flush(FSDataInputStream indexIn, FSDataInputStream dataIn, int flushPoint) throws IOException {
		this.flushPoint = flushPoint + 1;
		indexIn.seek(this.partition * JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);

		long segmentOffset    = indexIn.readLong();
		long rawSegmentLength = indexIn.readLong();
		long segmentLength    = indexIn.readLong();
				
		dataIn.seek(segmentOffset);
		flushFile(new IFileInputStream(dataIn, segmentLength), segmentLength, false);
	}
	
	private void flushFinal() throws IOException {
		MapOutputFile mapOutputFile = new MapOutputFile(this.taskid.getJobID());
		mapOutputFile.setConf(conf);

		Path finalOutputFile = mapOutputFile.getOutputFile(this.taskid);
		Path finalIndexFile = mapOutputFile.getOutputIndexFile(this.taskid);

		FSDataInputStream indexIn = localFS.open(finalIndexFile);
		indexIn.seek(this.partition * JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
		long segmentOffset    = indexIn.readLong();
		long rawSegmentLength = indexIn.readLong();
		long segmentLength    = indexIn.readLong();

		FSDataInputStream in = localFS.open(finalOutputFile);
		in.seek(segmentOffset);
		flushFile(new IFileInputStream(in, segmentLength), segmentLength, true);
	}
	
	@Override
	public void run() {
		synchronized (this) {
			try {
				flushFinal();
			} catch (IOException e) {
				e.printStackTrace();
			}
			finally {
				try {
					close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	
	public void open(Configuration conf) throws IOException {
		synchronized (this) {
			this.output = new byte[1500];
			this.conf = conf;
			this.localFS = FileSystem.getLocal(conf);
			this.out = connect();
			this.busy = false;
		}
	}
	
	private FSDataOutputStream connect() throws IOException {
		try {
			Socket socket = new Socket();
			socket.connect(this.sink);
			FSDataOutputStream out = new FSDataOutputStream(socket.getOutputStream());
			this.taskid.write(out);
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
			if (open) {
				open = false;
				out.flush();
				out.close();
			}
		}
	}
	
	private void flushFile(InputStream in, long length, boolean eof) throws IOException {
		synchronized (this) {
			while (busy) {
				try { this.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			busy = true;
		}
		
		out.writeLong(length);
		out.writeBoolean(eof);
			
		long bytes = length;
		while (open && bytes > 0) {
			int bytesread = in.read(output, 0, Math.min((int) bytes, output.length));
			out.write(output, 0, bytesread);
			bytes -= bytesread;
		}
		
		
		synchronized (this) {
			busy = false;
			this.notifyAll();
		}
	}

}
