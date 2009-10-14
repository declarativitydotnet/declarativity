package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapred.TaskAttemptID;

public class OutputFile implements Writable {
	public enum Type {FILE, SNAPSHOT};
	
	public abstract static class Header implements Writable, Comparable<Header> {
		private Type type;
		
		private TaskAttemptID owner;
		
		private float progress;
		
		public Header() {}
		
		public Header(Type type, TaskAttemptID owner, float progress) {
			this.type = type;
			this.owner = owner;
			this.progress = progress;
		}
		
		@Override
		public String toString() {
			return "Header: type " + type + 
			       " owner " + owner + 
			       " progress " + progress;
		}
		
		public Type type() { return type; }
		
		public TaskAttemptID owner() {  return owner;  }
		
		public float progress() { return this.progress; }
		
		@Override
		public void readFields(DataInput in) throws IOException {
			this.owner = new TaskAttemptID();
			this.owner.readFields(in);
			
			this.progress = in.readFloat();
		}

		@Override
		public void write(DataOutput out) throws IOException {
			this.owner.write(out);
			out.writeFloat(this.progress);
		}
		
		public static Header readHeader(DataInput in) throws IOException {
			Type type = WritableUtils.readEnum(in, Type.class);
			Header header = null;
			switch (type) {
			case FILE: header = new FileHeader(); break;
			case SNAPSHOT: header = new SnapshotHeader(); break;
			default: return null;
			}
			header.readFields(in);
			return header;
		}
		
		public static void writeHeader(DataOutput out, Header header) throws IOException {
			WritableUtils.writeEnum(out, header.type());
			header.write(out);
		}

	}
	
	public static class FileHeader extends Header {
		/* The name of the input file. */
		private String filename;
		
		/* The start offset. */
		private long start;
		
		/* The end offset. */
		private long end;
		
		/* The current position. */
		private int spillNum;
		
		public FileHeader() { super(Type.FILE, null, 0f); }
		
		public FileHeader(TaskAttemptID owner, float progress, String name, long start, long end, int spillNum) {
			super(Type.FILE, owner, progress);
			this.filename = name;
			this.start = start;
			this.end = end;
			this.spillNum = spillNum;
		}
		
		public String filename() { return this.filename; }
		public long start() { return this.start; }
		public long end() { return this.end; }
		public int spill() { return this.spillNum; }

		@Override
		public void readFields(DataInput in) throws IOException {
			super.readFields(in);
			
			this.filename = WritableUtils.readString(in);
			this.start = in.readLong();
			this.end = in.readLong();
			this.spillNum = in.readInt();
		}

		@Override
		public void write(DataOutput out) throws IOException {
			super.write(out);
			
			WritableUtils.writeString(out, this.filename);
			out.writeLong(this.start);
			out.writeLong(this.end);
			out.writeInt(this.spillNum);
		}

		@Override
		public int compareTo(Header o) {
			if (o.type() == this.type()) {
				return this.spillNum - ((FileHeader)o).spillNum;
			}
			return -1;
		}
	}
	
	public static class SnapshotHeader extends Header {
		
		public SnapshotHeader() { super(Type.SNAPSHOT, null, 0f); }
		
		public SnapshotHeader(TaskAttemptID owner, float progress) {
			super(Type.SNAPSHOT, owner, progress);
		}

		@Override
		public void readFields(DataInput in) throws IOException {
			super.readFields(in);
		}

		@Override
		public void write(DataOutput out) throws IOException {
			super.write(out);
		}

		@Override
		public int compareTo(Header o) {
			return progress() < o.progress() ? -1 :
					progress() > o.progress() ? 1 : 0;
		}
	}

	private Header header;
	
	private boolean complete;
	
	private Path data;
	
	private Path index;
	
	private FSDataInputStream dataIn = null;
	
	private FSDataInputStream indexIn = null;
	
	public OutputFile() { 	}
	
	public OutputFile(TaskAttemptID owner, float progress, Path snapData, Path snapIndex) {
		this.header = new SnapshotHeader(owner, progress);
		this.complete = progress == 1.0f;
		this.data = snapData;
		this.index = snapIndex;
	}
	
	public OutputFile(TaskAttemptID owner, float progress, String input, long start, long end, int spillNum, Path data, Path index, boolean complete) {
		this.header = new FileHeader(owner, progress, input, start, end, spillNum);
		this.data     = data;
		this.index    = index;
		this.complete = complete;
	}
	
	public Header header() {
		return this.header;
	}
	
	public Path data() {
		return this.data;
	}
	
	public Path index() {
		return this.index;
	}
	
	public boolean complete() {
		return this.complete;
	}
	
	public void open(FileSystem localFs) throws IOException {
		if (this.dataIn == null) {
			this.dataIn = localFs.open(data());
			this.indexIn = localFs.open(index());
		}
	}
	
	public void close() throws IOException {
		if (this.dataIn != null) {
			this.dataIn.close();
			this.dataIn = null;
		}
		
		if (this.indexIn != null) {
			this.indexIn.close();
			this.indexIn = null;
		}
	}
	
	/**
	 * Seek to partition.
	 * @param partition # to seek to.
	 * @return parition segment length.
	 * @throws IOException
	 */
	public long seek(int partition) throws IOException {
		indexIn.seek(partition * JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
		long segmentOffset    = indexIn.readLong();
		long rawSegmentLength = indexIn.readLong();
		long segmentLength    = indexIn.readLong();

		dataIn.seek(segmentOffset);
		return segmentLength;
	}
	
	public FSDataInputStream dataInputStream() {
		return this.dataIn;
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.header = Header.readHeader(in);
		
		String dataStr = WritableUtils.readString(in);
		String indexStr = WritableUtils.readString(in);
		this.data = new Path(dataStr);
		this.index = new Path(indexStr);
		
	}

	@Override
	public void write(DataOutput out) throws IOException {
		Header.writeHeader(out, this.header);
		
		WritableUtils.writeString(out, this.data.toString());
		WritableUtils.writeString(out, this.index.toString());
	}
	
}
