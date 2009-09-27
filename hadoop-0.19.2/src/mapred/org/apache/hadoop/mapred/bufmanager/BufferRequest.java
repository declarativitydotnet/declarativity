package org.apache.hadoop.mapred.bufmanager;

import java.io.BufferedOutputStream;
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
import org.apache.hadoop.mapred.IFileOutputStream;
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
	
	public transient int connectionAttempts = 1;
	
	public boolean delivered = false;
	
	private TaskAttemptID taskid;
	
	private int partition;
	
	private String source = null;
	
	private InetSocketAddress sink = null;
	
	private FSDataOutputStream out = null;
	
	private JobConf conf = null;
	
	private FileSystem localFS = null;
	
	private int flushPoint = -1;
	
	private float datarate = 0f;
	
	public BufferRequest() {
	}
	
	public BufferRequest(TaskAttemptID taskid, int partition, String source, InetSocketAddress sink) {
		this.taskid    = taskid;
		this.partition = partition;
		this.source = source;
		this.sink   = sink;
	}
	
	@Override
	public int hashCode() {
		String code = taskid.toString() + ":" + partition;
		return code.hashCode();
	}
	
	@Override
	public int compareTo(BufferRequest o) {
		if (this.taskid.compareTo(o.taskid) != 0) {
			return this.taskid.compareTo(o.taskid);
		}
		else {
			return this.partition - o.partition;
		}
	}
	
	@Override
	public boolean equals (Object o) {
		if (o instanceof BufferRequest) {
			if (compareTo((BufferRequest) o) == 0) {
				/* and it's going to the same place */
				return this.sink.equals(((BufferRequest)o).sink);
			}
		}
		return false;
	}
	
	@Override
	public String toString() {
		return "BufferRequest: task " + taskid + " partition " + partition + " from " + source + " to " + sink;
	}
	
	public Integer partition() {
		return this.partition;
	}
	
	public int flushPoint() {
		return this.flushPoint;
	}
	
	public float datarate() {
		return this.datarate;
	}
	
	public void flush(FSDataInputStream indexIn, FSDataInputStream dataIn, float progress) throws IOException {
		flush(indexIn, dataIn, -1, progress);
	}
	
	public synchronized void flush(FSDataInputStream indexIn, FSDataInputStream dataIn, int flushPoint, float progress) throws IOException {
		indexIn.seek(this.partition * JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);

		long segmentOffset    = indexIn.readLong();
		long rawSegmentLength = indexIn.readLong();
		long segmentLength    = indexIn.readLong();

		dataIn.seek(segmentOffset);
		try {
			flushFile(dataIn, segmentLength, progress);
		} catch (IOException e) {
			close();
			throw e;
		}
		this.flushPoint = flushPoint;
	}
	
	private synchronized void flushFinal() throws IOException {
		MapOutputFile mapOutputFile = new MapOutputFile(this.taskid.getJobID());
		mapOutputFile.setConf(conf);

		Path finalOutputFile = mapOutputFile.getOutputFile(this.taskid);
		Path finalIndexFile = mapOutputFile.getOutputIndexFile(this.taskid);
		
		try {
		if (!localFS.exists(finalOutputFile)) {
			throw new IOException("BufferRequest: file does not exist! " + finalOutputFile);
		}
		} catch (IOException e ) {
			e.printStackTrace();
			throw e;
		}

		FSDataInputStream indexIn = localFS.open(finalIndexFile);
		indexIn.seek(this.partition * JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
		long segmentOffset    = indexIn.readLong();
		long rawSegmentLength = indexIn.readLong();
		long segmentLength    = indexIn.readLong();
		indexIn.close();

		FSDataInputStream in = localFS.open(finalOutputFile);
		in.seek(segmentOffset);
		flushFile(in, segmentLength, 1.0f);
		
		in.close();
	}
	
	@Override
	public void run() {
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
	
	
	public void open(JobConf conf, BufferRequestResponse response, boolean snapshot) {
		this.conf = conf;
		int connectAttempts = snapshot ? 1 : this.conf.getInt("mapred.connection.attempts", 5);
		open(conf, response, snapshot, connectAttempts);
	}
	
	public void open(JobConf conf, BufferRequestResponse response, boolean snapshot, int connectAttempts) {
		this.conf = conf;
		this.connectionAttempts = connectAttempts;
		try {
			this.localFS = FileSystem.getLocal(conf);
		} catch (IOException e) {
			System.err.println("BufferRequest unable to get FileSystem!");
			this.out = null;
			return;
		}
		
		synchronized (this) {
			response.reset();
			this.out = connect(response, snapshot);
		}
	}
	
	private FSDataOutputStream connect(BufferRequestResponse response, boolean snapshot) {
		Socket socket = new Socket();
		try {
			for (int i = 0; i < connectionAttempts && !socket.isConnected(); i++) {
				try {
					socket.connect(this.sink);
				} catch (IOException e) {
					System.err.println("BufferRequest: " + e);
					if (i == connectionAttempts) {
						response.setRetry();
						return null;
					}
				}
			}
			FSDataOutputStream out = new FSDataOutputStream(new BufferedOutputStream(socket.getOutputStream()));
			this.taskid.write(out);
			out.writeBoolean(snapshot);
			out.flush();
			
			DataInputStream in = new DataInputStream(socket.getInputStream());
			response.readFields(in);
			if (!response.open) {
				out.close();
				return null;
			}
			return out;
		} catch (Throwable e) {
			try { if (!socket.isClosed()) socket.close();
			} catch (Throwable t) { }
			return null;
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
		/* The taskid */
		this.taskid = new TaskAttemptID();
		this.taskid.readFields(in);
		
		/* The partition */
		this.partition = WritableUtils.readVInt(in);
		
		/* The address on which the map executes */
		this.source = WritableUtils.readString(in);
		
		/* The address on which the reduce executes */
		String sinkHost = WritableUtils.readString(in);
		int    sinkPort = WritableUtils.readVInt(in);
		this.sink = new InetSocketAddress(sinkHost, sinkPort);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		if (this.source == null || this.sink == null)
			throw new IOException("No source/sink in request!");
		
		/* The taskid */
		this.taskid.write(out);
		
		/* The partition */
		WritableUtils.writeVInt(out, this.partition);
		
		/* The address on which the map executes */
		WritableUtils.writeString(out, this.source);
		
		/* The address on which the reduce executes */
		WritableUtils.writeString(out, this.sink.getHostName());
		WritableUtils.writeVInt(out, this.sink.getPort());
	}

	public void close() throws IOException {
		synchronized (this) {
			try {
				if (out != null) {
					out.flush();
					out.close();
					out = null;
				}
			} catch (Throwable t) {
				/* Ignore. */
			}
		}
	}
	
	public boolean isOpen() {
		synchronized (this) {
			return out != null;
		}
	}
	
	public IFile.Writer<K, V> force(DataInputBuffer key, DataInputBuffer value, IFile.Writer<K, V> writer, long records, float progress) throws IOException {
		synchronized (this) {
			if (!isOpen()) throw new IOException("BufferRequest is closed!");

			if (writer == null) {
				System.err.println("Request " + this + " create new forced writer for " + records + 
						           " records at progress = " + progress + ".");
				CompressionCodec codec = null;
				if (conf.getCompressMapOutput()) {
					Class<? extends CompressionCodec> codecClass =
						conf.getMapOutputCompressorClass(DefaultCodec.class);
					codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
				}
				Class <K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
				Class <V> valClass = (Class<V>)conf.getMapOutputValueClass();
				
				out.writeBoolean(true);
				out.writeLong(records);
				out.writeFloat(progress);
				writer = new IFile.Writer<K, V>(conf, out,  keyClass, valClass, codec);
			}
			writer.append(key, value);
			return writer;
		}
	}
	
	private void flushFile(FSDataInputStream in, long length, float progress) throws IOException {
		synchronized (this) {
			if (length == 0 && progress < 1.0f) {
				return;
			}
			else if (!isOpen()) throw new IOException("BufferRequest is closed!");

			CompressionCodec codec = null;
			if (conf.getCompressMapOutput()) {
				Class<? extends CompressionCodec> codecClass =
					conf.getMapOutputCompressorClass(DefaultCodec.class);
				codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
			}
			Class <K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
			Class <V> valClass = (Class<V>)conf.getMapOutputValueClass();

			long starttime = System.currentTimeMillis();
			out.writeBoolean(false);
			out.writeLong(length);
			out.writeFloat(progress);

			IFile.Reader reader = new IFile.Reader<K, V>(conf, in, length, codec);
			IFile.Writer writer = new IFile.Writer<K, V>(conf, out,  keyClass, valClass, codec);

			try {
				DataInputBuffer key = new DataInputBuffer();
				DataInputBuffer value = new DataInputBuffer();
				while (reader.next(key, value)) {
					writer.append(key, value);
				}
			} finally {
				out.flush();
				writer.close();
			}

			if (length > 0) {
				long  stoptime = System.currentTimeMillis();
				float duration = 1.0f + (stoptime - starttime);
				float rate = ((float) length) / duration;
				datarate = (0.75f * rate) + (0.25f * datarate);
			}
		}
	}

}
