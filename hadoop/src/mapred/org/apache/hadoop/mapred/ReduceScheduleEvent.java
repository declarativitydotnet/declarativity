package org.apache.hadoop.mapred;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.Comparator;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;

public class ReduceScheduleEvent implements Writable, Comparator<ReduceScheduleEvent> {

	private TaskID taskid;

	private Integer partition;

	private String address;

	public ReduceScheduleEvent() {}

	public ReduceScheduleEvent(TaskID taskid, int partition, String address) {
		this.taskid = taskid;
		this.partition = partition;
		this.address = address;
	}

	@Override
	public String toString() {
		return taskid != null ? 
				(taskid.toString() + ":" + partition).toString() : "null";
	}

	@Override
	public int hashCode() {
		return toString().hashCode();
	}

	@Override
	public boolean equals(Object o) {
		if (o instanceof ReduceScheduleEvent) {
			return toString().equals(o.toString());
		}
		return false;
	}
	
	@Override
	public int compare(ReduceScheduleEvent o1, ReduceScheduleEvent o2) {
		return o1.partition.compareTo(o2.partition);
	}

	public TaskID getTaskID() {
		return this.taskid;
	}

	public int getPartition() {
		return this.partition;
	}

	public String getAddress() {
		return this.address;
	}
	
	public SocketAddress getSocketAddress() {
		if (this.address == null) return null;
		int divider = this.address.indexOf(':');
		if (divider < 0) return null;
		
		String host = this.address.substring(0, divider);
		int port = Integer.parseInt(this.address.substring(divider+1));
		return new InetSocketAddress(host, port);
	}

	//////////////////////////////////////////////
	// Writable
	//////////////////////////////////////////////
	public void write(DataOutput out) throws IOException {
		taskid.write(out); 
		WritableUtils.writeVInt(out, partition);
		WritableUtils.writeString(out, address);
	}

	public void readFields(DataInput in) throws IOException {
		this.taskid = TaskID.read(in); 
		this.partition = WritableUtils.readVInt(in);
		this.address = WritableUtils.readString(in);
	}

}
