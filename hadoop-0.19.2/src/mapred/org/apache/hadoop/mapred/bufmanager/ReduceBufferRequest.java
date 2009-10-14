package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.net.InetSocketAddress;

import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public class ReduceBufferRequest extends BufferRequest {
	
	private TaskID reduceTaskId;
	
	public ReduceBufferRequest() {
	}
	
	public ReduceBufferRequest(String sourceHost, TaskAttemptID destTaskId, InetSocketAddress destinationAddress, TaskID reduceTaskId) {
		super(Type.REDUCE, sourceHost, destTaskId, destinationAddress);
		this.reduceTaskId = reduceTaskId;
	}
	
	@Override
	public void readFields(DataInput in) throws IOException {
		super.readFields(in);
		this.reduceTaskId = new TaskID();
		this.reduceTaskId.readFields(in);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		super.write(out);
		this.reduceTaskId.write(out);
	}
	
	public TaskID reduceTaskId() {
		return this.reduceTaskId;
	}

	@Override
	public int partition() {
		return 0;
	}

}
