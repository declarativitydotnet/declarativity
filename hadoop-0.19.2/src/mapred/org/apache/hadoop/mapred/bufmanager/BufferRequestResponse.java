package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class BufferRequestResponse implements Writable {
	
	/* Buffer request is open. */
	public boolean open;
	
	/* Buffer request has been terminated. 
	 * (e.g., no longer needed). */
	public boolean terminated;
	
	/* Buffer receiver requests blocking mode. */
	public boolean blocking;
	
	/* Buffer receiver requests starting
	 * from this offset. */
	public long offset;

	public BufferRequestResponse() { reset(); }
	
	public void reset() {
		this.open = false;
		this.terminated = false;
		this.blocking = false;
		this.offset = 0;
	}
	
	public void setBlocking(long offset) {
		reset();
		this.open = true;
		this.blocking = true;
		this.offset = offset;
	}
	
	public void setOpen() {
		reset();
		this.open = true;
	}
	
	public void setRetry() {
		reset();
	}
	
	public void setTerminated() {
		reset();
		terminated = true;
	}
	
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
