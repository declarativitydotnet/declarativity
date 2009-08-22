package org.apache.hadoop.mapred;

import java.io.IOException;

import org.apache.hadoop.fs.Path;

public class ReduceOutputFile extends MapOutputFile {

	private TaskAttemptID reduceID;

	public ReduceOutputFile(TaskAttemptID reduceID) {
		super(reduceID.getJobID());
		this.reduceID = reduceID;
	}

	@Override
	public Path getOutputFile(TaskAttemptID mapTaskId)
	throws IOException {
		return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ "/file.out", conf);
	}

	@Override
	public Path getOutputFileForWrite(TaskAttemptID mapTaskId, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ "/file.out", size, conf);
	}

	@Override
	public Path getOutputIndexFile(TaskAttemptID mapTaskId)
	throws IOException {
		return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ "/file.out.index", conf);
	}

	@Override
	public Path getOutputIndexFileForWrite(TaskAttemptID mapTaskId, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ "/file.out.index", 
				size, conf);
	}
}
