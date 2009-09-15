package org.apache.hadoop.mapred;

import java.io.IOException;

import org.apache.hadoop.mapred.bufmanager.JBuffer;
import org.apache.hadoop.util.ReflectionUtils;

public class SnapshotMapRunner<K1, V1, K2, V2> 
       extends MapRunner<K1, V1, K2, V2> {
	
	private int snapshotIntervals;
	
	private float threshold;

	
	  @SuppressWarnings("unchecked")
	  public void configure(JobConf job) {
		  super.configure(job);
		  snapshotIntervals = job.getInt("mapred.snapshot.interval", 2);
		  this.threshold = 0f;
		  checkInterval(0f);
	  }
	  
	  private boolean checkInterval(float progress) {
		  if (progress >= threshold) {
			  this.threshold += (1f / (float) snapshotIntervals);
			  return true;
		  }
		  return false;
	  }

	  public void 
	  run(RecordReader<K1, V1> input, OutputCollector<K2, V2> output, Reporter reporter)
	  throws IOException {
		  try {
			  JBuffer<K2, V2> buffer = null;
			  if (output instanceof JBuffer) {
				  buffer = (JBuffer<K2, V2>) output;
			  }
			  else {
				  throw new IOException("SnapshotMapRunner: expects output buffer to be JBuffer!");
			  }

			  // allocate key & value instances that are re-used for all entries
			  K1 key = input.createKey();
			  V1 value = input.createValue();

			  boolean doSnapshots = true;
			  while (input.next(key, value)) {
				  // map pair to output
				  mapper.map(key, value, output, reporter);
				  if(incrProcCount) {
					  reporter.incrCounter(SkipBadRecords.COUNTER_GROUP, 
							  SkipBadRecords.COUNTER_MAP_PROCESSED_RECORDS, 1);
				  }
				  
				  if (doSnapshots && checkInterval(input.getProgress())) {
					  doSnapshots = buffer.snapshot(false);
				  }
			  }
		  } finally {
			  mapper.close();
		  }
	  }
}
