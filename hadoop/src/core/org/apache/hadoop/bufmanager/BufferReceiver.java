package org.apache.hadoop.bufmanager;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;

import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.Decompressor;
import org.apache.hadoop.mapred.TaskAttemptID;

public class BufferReceiver implements Runnable {
	
	private Buffer buffer;
	
	private DataInputStream input;
    
	public BufferReceiver(Buffer buffer, DataInputStream input) {
		this.buffer = buffer;
		this.input  = input;
	}
	
	@Override
	public void run() {
		try {
			while (true) {
				Record record = new Record();
				record.readFields(this.input);
				if (record.isNull()) break;
				this.buffer.add(record);
			}
		} catch (IOException e) {
			System.err.println("Exception Buffer Reciever: " + e);
		}
		finally {
			buffer.done(this);
			try {
				this.input.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	public void cancel() throws IOException {
		this.input.close();
	}
}
