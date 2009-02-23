package org.apache.hadoop.mapred.declarative.util;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.mapred.JobClient;

public class FileInput {
	
	private Path path;
	
	private JobClient.RawSplit split;
	
	public FileInput(Path path, JobClient.RawSplit split) {
		if (split == null) System.err.println("FILE INPUT WITH NULL SPLIT!");
		this.path = path;
		this.split = split;
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof FileInput) {
			FileInput other = (FileInput) o;
			return this.split.equals(other.split);
		}
		return false;
	}
	
	@Override
	public String toString() {
		return this.path != null ? this.path.toString() : split.toString();
	}
	
	public Path path() {
		return this.path;
	}
	
	public JobClient.RawSplit split() {
		return this.split;
	}

}
