package org.apache.hadoop.bufmanager;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;

import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.Decompressor;
import org.apache.hadoop.mapred.TaskAttemptID;

public class BufferReceiver implements Runnable {
	
	private BufferManager manager;
	
	private Buffer buffer;
	
	private TaskAttemptID tid;
	
	private Socket socket;
	
    private CompressionCodec codec;
    
	public BufferReceiver(BufferManager manager, Buffer buffer, TaskAttemptID tid, Socket socket, CompressionCodec codec) {
		this.manager = manager;
		this.buffer = buffer;
		this.tid    = tid;
		this.socket = socket;
		this.codec  = codec;
	}
	
	public Buffer buffer() {
		return this.buffer;
	}
	
	public TaskAttemptID tid() {
		return this.tid;
	}

	@Override
	public void run() {
		DataInputStream in = null;
		try {
			if (codec != null) {
				Decompressor decompressor = CodecPool.getDecompressor(codec);
				decompressor.reset();
				in = new DataInputStream(codec.createInputStream(socket.getInputStream(), decompressor));
			}
			else {
				in = new DataInputStream(socket.getInputStream());
			}
			
			int records = 0;
			while (!socket.isClosed()) {
				Record record = new Record();
				record.readFields(in);
				record.unmarshall(this.buffer.conf());
				records++;
				this.manager.add(buffer.bufid(), record);
			}
			System.err.println("BufferReceiver: received " + records + " records.");
			
		} catch (java.io.EOFException e) {
			// Cool.
		} catch (IOException e) {
			System.err.println("Exception Buffer Reciever: " + e);
		}
		finally {
			if (in != null) {
				try {  in.close();
				} catch (IOException e) { }
			}
			System.err.println("BufferReceiver done -- " + this.buffer.bufid());
			this.manager.done(this);
		}
	}
	
	public void cancel() throws IOException {
		this.socket.close();
	}
}
