package org.apache.hadoop.mapred;

import java.io.IOException;

import org.apache.hadoop.fs.Path;

public class ReduceOutputFile extends MapOutputFile {

	private TaskAttemptID reduceID;

	public ReduceOutputFile(TaskAttemptID reduceID) {
		super(reduceID.getJobID());
		this.reduceID = reduceID;
	}

	public Path getOutputFileForWrite(TaskAttemptID mapTaskId, int spillid, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ "_" + spillid + "_" + "file.out", size, conf);
	}

	public Path getOutputIndexFileForWrite(TaskAttemptID mapTaskId, int spillid, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ "_" + spillid + "_" + "file.out.index", 
				size, conf);
	}
	
	
	public Path getOutputRunFileForWrite(TaskID id, int run, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + id.toString()
				+ "_run_" + run + "_file.out", size, conf);
	}

	public Path getOutputRunIndexFileForWrite(TaskID id, int run, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + id.toString()
				+ "_run_"  + run + "_file.out.index", 
				size, conf);
	}
}
