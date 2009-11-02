package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.net.InetSocketAddress;

import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public class MapBufferRequest extends BufferRequest {
	
	private JobID mapJobId;
	
	private int mapPartition;
	
	private String code;
	
	public MapBufferRequest() {
	}
	
	public MapBufferRequest(String sourceHost, TaskAttemptID destTaskId, InetSocketAddress destinationAddress, JobID mapJobId, int mapPartition) {
		super(sourceHost, destTaskId, destinationAddress);
		this.mapJobId = mapJobId;
		this.mapPartition = mapPartition;
		this.code = sourceHost + ":" + destTaskId + ":" + mapJobId + ":" + mapPartition;
	}
	
	@Override
	public int hashCode() {
		return code.hashCode();
	}
	
	public boolean equals(Object o) {
		if (o instanceof MapBufferRequest) {
			return this.code.equals(((MapBufferRequest)o).code);
		}
		return false;
	}
	
	@Override
	public String toString() {
		return destination() + " requesting map buffers from job " + mapJobId.toString() + " partition " + mapPartition;
	}
	
	@Override
	public void readFields(DataInput in) throws IOException {
		super.readFields(in);
		this.mapJobId = new JobID();
		this.mapJobId.readFields(in);
		this.mapPartition = in.readInt();
		this.code = WritableUtils.readString(in);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		super.write(out);
		this.mapJobId.write(out);
		out.writeInt(this.mapPartition);
		WritableUtils.writeString(out, this.code);
	}
	
	public JobID mapJobId() {
		return this.mapJobId;
	}
	
	public int partition() {
		return this.mapPartition;
	}

	@Override
	public Type type() {
		return Type.MAP;
	}
}
