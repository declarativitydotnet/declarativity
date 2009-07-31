package org.apache.hadoop.bufmanager;

import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.apache.hadoop.fs.FSDataOutputStream;
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
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferRequest implements Writable, Runnable {
	private BufferID bufid;
	
	private String source;
	
	private InetSocketAddress sink;
	
	private JobConf conf = null;
	
	private Buffer buffer = null;
	
	public BufferRequest() {}
	
	public BufferRequest(BufferID bufid, String source, InetSocketAddress sink) {
		this.bufid  = bufid;
		this.source = source;
		this.sink   = sink;
	}
	
	public void open(JobConf conf, Buffer buffer) {
		this.conf = conf;
		this.buffer = buffer;
	}
	
	public BufferID bufid() {
		return this.bufid;
	}
	
	public String source() {
		return this.source;
	}
	
	public InetSocketAddress sink() {
		return this.sink;
	}
	
	@Override
	public void readFields(DataInput in) throws IOException {
		this.bufid = new BufferID();
		this.bufid.readFields(in);
		this.source = WritableUtils.readString(in);
		
		String sinkHost = WritableUtils.readString(in);
		int    sinkPort = WritableUtils.readVInt(in);
		this.sink = new InetSocketAddress(sinkHost, sinkPort);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		this.bufid.write(out);
		WritableUtils.writeString(out, this.source);
		
		WritableUtils.writeString(out, this.sink.getHostName());
		WritableUtils.writeVInt(out, this.sink.getPort());
	}

	@Override
	public void run() {
		Socket socket = new Socket();
		try {
			socket.connect(this.sink);
		} catch (IOException e) {
			e.printStackTrace();
			return;
		}
		
		DataOutputStream out = null;
		try {
			if (conf.getCompressMapOutput()) {
				Class<? extends CompressionCodec> codecClass =
					conf.getMapOutputCompressorClass(DefaultCodec.class);
				CompressionCodec codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
				Compressor compressor = CodecPool.getCompressor(codec);
				compressor.reset();
				OutputStream compressedOut = codec.createOutputStream(socket.getOutputStream(), compressor);
				out = new DataOutputStream(compressedOut);
			} else {
				out = new DataOutputStream(socket.getOutputStream());
			}
			
			Iterator<Record> iterator = this.buffer.iterator();
			while (iterator.hasNext()) {
				Record record = iterator.next();
				record.write(out);
			}
			Record.NULL_RECORD.write(out);
		} catch (IOException e) { e.printStackTrace(); }
		finally {
			this.buffer.done(this);
			try {
				out.flush();
				out.close();
			} catch (IOException e) { e.printStackTrace(); }
		}
	}
}
