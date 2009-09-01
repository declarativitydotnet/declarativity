package org.apache.hadoop.mapred;

import java.io.IOException;

import org.apache.hadoop.fs.Path;

public class ReduceOutputFile extends MapOutputFile {

	private TaskAttemptID reduceID;

	public ReduceOutputFile(TaskAttemptID reduceID) {
		super(reduceID.getJobID());
		this.reduceID = reduceID;
	}

	public Path getOutputFile(TaskAttemptID mapTaskId, boolean eof)
	throws IOException {
		return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ (eof ? "_eof_" : "") + "file.out", conf);
	}

	public Path getOutputFileForWrite(TaskAttemptID mapTaskId, boolean eof, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ (eof ? "_eof_" : "") + "file.out", size, conf);
	}

	public Path getOutputIndexFile(TaskAttemptID mapTaskId, boolean eof)
	throws IOException {
		return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ (eof ? "_eof_" : "") + "file.out.index", conf);
	}

	public Path getOutputIndexFileForWrite(TaskAttemptID mapTaskId, boolean eof, long size)
	throws IOException {
		return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
				jobId.toString(), reduceID.toString())
				+ Path.SEPARATOR + mapTaskId.toString()
				+ (eof ? "_eof_" : "") + "file.out.index", 
				size, conf);
	}
}
