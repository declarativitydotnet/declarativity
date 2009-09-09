package org.apache.hadoop.mapred;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

class ReduceTaskCompletionEventsUpdate extends MapTaskCompletionEventsUpdate {

	public ReduceTaskCompletionEventsUpdate() {};

	public ReduceTaskCompletionEventsUpdate(TaskCompletionEvent[] reduceEvents, boolean b) {
		super(reduceEvents, b);
	}

	public void write(DataOutput out) throws IOException {
		super.write(out);
	}

	public void readFields(DataInput in) throws IOException {
		super.readFields(in);

	}
}
