package org.apache.hadoop.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public class BufferID implements Writable {
	
	private TaskAttemptID taskid;
	
	private Integer partition;
	
	public BufferID() {}
	
	public BufferID(TaskAttemptID taskid, Integer partition) {
		this.taskid = taskid;
		this.partition = partition;
	}
	
	public String toString() {
		return taskid != null ? 
				"buffer:" + taskid.toString() + ":" + partition : "null";
	}
	
	public int hashCode() {
		return toString().hashCode();
	}
	
	public boolean equals(Object o) {
		if (o instanceof BufferID) {
			BufferID other = (BufferID) o;
			return this.taskid.equals(other.taskid) &&
					this.partition.equals(other.partition);
		}
		return false;
	}
	
	public TaskAttemptID taskid() {
		return this.taskid;
	}
	
	public Integer partition() {
		return this.partition;
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.taskid = new TaskAttemptID();
		this.taskid.readFields(in);
		this.partition = in.readInt();
	}

	@Override
	public void write(DataOutput out) throws IOException {
		if (this.taskid != null) {
			this.taskid.write(out);
			out.writeInt(this.partition);
		}
	}

}
