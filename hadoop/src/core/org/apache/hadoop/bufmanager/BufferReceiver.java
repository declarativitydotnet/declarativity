package org.apache.hadoop.bufmanager;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;

import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.Decompressor;
import org.apache.hadoop.mapred.TaskAttemptID;

public class BufferReceiver implements Runnable {
	
	private BufferID requestID;
	
	private BufferTransfer buffer;
	
	private DataInputStream input;
	
	public boolean open;
    
	public BufferReceiver(BufferID requestID, BufferTransfer buffer, DataInputStream input) {
		this.requestID = requestID;
		this.buffer = buffer;
		this.input  = input;
		this.open = true;
	}
	
	public BufferID requestID() {
		return this.requestID;
	}
	
	@Override
	public void run() {
		try {
			System.err.println("BufferReceiver: start request " + requestID);
			while (this.open) {
				Record record = new Record();
				record.readFields(this.input);
				if (record.isNull()) break;
				this.buffer.add(record);
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		finally {
			System.err.println("BufferReceiver: done request " + requestID);
			buffer.done(this);
			close();
		}
	}
	
	public void close() {
		try {
			if (this.open) {
				this.open = false;
				this.input.close();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
