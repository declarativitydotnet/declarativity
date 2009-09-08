package org.apache.hadoop.mapred;

class ReduceTaskCompletionEventsUpdate extends MapTaskCompletionEventsUpdate {

	public ReduceTaskCompletionEventsUpdate(TaskCompletionEvent[] reduceEvents, boolean b) {
		super(reduceEvents, b);
	}

}
