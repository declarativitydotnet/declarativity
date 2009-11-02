package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.net.InetSocketAddress;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public abstract class BufferRequest implements Writable {
	public static enum Type{MAP, REDUCE};
	
	protected String srcHost;
	
	protected TaskAttemptID destTaskId;
	
	protected InetSocketAddress destAddress;
	
	public BufferRequest() {
	}
	
	public BufferRequest(String sourceHost, TaskAttemptID destTaskId, InetSocketAddress destinationAddress) {
		this.srcHost = sourceHost;
		this.destTaskId = destTaskId;
		this.destAddress = destinationAddress;
	}
	
	
	public static BufferRequest read(DataInput in) throws IOException {
		Type type = WritableUtils.readEnum(in, Type.class);
		BufferRequest request = null;
		if (type == Type.MAP) {
			request = new MapBufferRequest();
		}
		else {
			request = new ReduceBufferRequest();
		}
		request.readFields(in);
		return request;
	}
	
	public static void write(DataOutput out, BufferRequest request) throws IOException {
		WritableUtils.writeEnum(out, request.type());
		request.write(out);
	}
	
	@Override
	public void readFields(DataInput in) throws IOException {
		/* The hostname on which the source executes */
		this.srcHost = WritableUtils.readString(in);
		
		/* The destination task id. */
		this.destTaskId = new TaskAttemptID();
		this.destTaskId.readFields(in);
		
		/* The address on which the destination executes */
		String host = WritableUtils.readString(in);
		int    port = WritableUtils.readVInt(in);
		this.destAddress = new InetSocketAddress(host, port);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		/* The hostname on which the source executes */
		WritableUtils.writeString(out, this.srcHost);
		
		/* The destination task id. */
		this.destTaskId.write(out);
		
		/* The address on which the destination executes */
		WritableUtils.writeString(out, destAddress.getHostName());
		WritableUtils.writeVInt(out, destAddress.getPort());
	}
	
	public abstract Type type();
	
	public abstract int partition();
	
	public TaskAttemptID destination() {
		return this.destTaskId;
	}
	
	public InetSocketAddress destAddress() {
		return this.destAddress;
	}
	
	public String srcHost() {
		return this.srcHost;
	}
}
