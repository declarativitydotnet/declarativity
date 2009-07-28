package org.apache.hadoop.bufmanager;

import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Iterator;

import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.Compressor;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferRequest implements Writable, Runnable {
	
	private BufferID bufid;
	
	private Buffer buffer;
	
	private Socket socket;
	
	// Compression for map-outputs
	private CompressionCodec codec = null;

	public BufferRequest() {}
	
	public BufferRequest(BufferID bufid) {
		this.bufid = bufid;
	}
	
	public BufferID bufid() {
		return this.bufid;
	}
	
	public void initialize(BufferManager.BufferControl buffer, Socket socket) {
		this.buffer = buffer;
		this.socket = socket;
		this.codec  = (buffer != null) ? buffer.codec() : null;
	}
	
	public Socket socket() {
		return this.socket;
	}
	
	@Override
	public void readFields(DataInput in) throws IOException {
		this.bufid = new BufferID();
		this.bufid.readFields(in);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		this.bufid.write(out);
	}

	@Override
	public void run() {
		DataOutputStream out = null;
		try {
			if (codec != null) {
				Compressor compressor = CodecPool.getCompressor(codec);
				compressor.reset();
				OutputStream compressedOut = codec.createOutputStream(this.socket.getOutputStream(), compressor);
				out = new DataOutputStream(compressedOut);
			} else {
				out = new DataOutputStream(this.socket.getOutputStream());
			}
			
			int records = 0;
			Iterator<Record> iter = this.buffer.records();
			while (iter.hasNext()) {
				Record record = iter.next();
				records++;
				record.write(out);
			}
			
			System.err.println("BufferRequest: sent " + records + " records.");
			
		} catch (IOException e) { e.printStackTrace(); }
		finally {
			try {
				out.flush();
				this.socket.close();
			} catch (IOException e) { e.printStackTrace(); }
		}
	}

}
