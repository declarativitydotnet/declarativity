package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class BufferRequestResponse implements Writable {
	
	/* Buffer request is open. */
	public boolean open = true;
	
	/* Buffer request has been terminated. 
	 * (e.g., no longer needed). */
	public boolean terminated = false;
	
	/* Buffer receiver requests blocking mode. */
	public boolean blocking = false;
	
	/* Buffer receiver requests starting
	 * from this offset. */
	public long offset = 0;

	public BufferRequestResponse() { }
	
	
	@Override
	public void readFields(DataInput in) throws IOException {
		this.open       = in.readBoolean();
		this.terminated = in.readBoolean();
		this.blocking   = in.readBoolean();
		this.offset     = in.readLong();
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeBoolean(open);
		out.writeBoolean(terminated);
		out.writeBoolean(blocking);
		out.writeLong(offset);
	}
}
