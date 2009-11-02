package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.net.InetSocketAddress;

import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public class ReduceBufferRequest extends BufferRequest {
	
	private TaskID reduceTaskId;
	
	private String code;
	
	public ReduceBufferRequest() {
	}
	
	public ReduceBufferRequest(String sourceHost, TaskAttemptID mapTaskId, InetSocketAddress destinationAddress, TaskID reduceTaskId) {
		super(sourceHost, mapTaskId, destinationAddress);
		this.reduceTaskId = reduceTaskId;
		
		this.code = sourceHost + ":" + mapTaskId + ":" + reduceTaskId;
	}
	
	@Override
	public int hashCode() {
		return code.hashCode();
	}
	
	public boolean equals(Object o) {
		if (o instanceof ReduceBufferRequest) {
			return this.code.equals(((ReduceBufferRequest)o).code);
		}
		return false;
	}
	
	@Override
	public String toString() {
		return destination() + " requesting buffer from reduce " + this.reduceTaskId.toString();
	}
	
	@Override
	public void readFields(DataInput in) throws IOException {
		super.readFields(in);
		this.reduceTaskId = new TaskID();
		this.reduceTaskId.readFields(in);
		this.code = WritableUtils.readString(in);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		super.write(out);
		this.reduceTaskId.write(out);
		WritableUtils.writeString(out, this.code);
	}
	
	public TaskID reduceTaskId() {
		return this.reduceTaskId;
	}

	@Override
	public int partition() {
		return 0;
	}

	@Override
	public Type type() {
		return Type.REDUCE;
	}

}
