package org.apache.hadoop.mapred.bufmanager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.mapred.FileHandle;
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.util.ReflectionUtils;

public class SnapshotManager<K extends Object, V extends Object> {
	private static final Log LOG = LogFactory.getLog(SnapshotManager.class.getName());

	public class Snapshot {
		public boolean fresh = false;
		
		private TaskID taskid;
		
		private Path data  = null;
		private Path index = null;
		
		private long length = 0;
		
		private float progress = 0f;
		
		private int runs = 0;
		
		public Snapshot(TaskID taskid) {
			this.taskid = taskid;
		}
		
		public String toString() {
			return "JBufferSnapshot " + taskid + ": progress = " + progress;
		}
		
		public boolean valid() {
			return this.data != null && length > 0;
		}
		
		/**
		 * Spill the current snapshot to the buffer.
		 * This method ensures that a consistent snapshot is spilled to
		 * the buffer. We don't want the next snapshot interfering with
		 * the buffer spill.
		 * @param buffer
		 * @throws IOException
		 */
		public void spill(JBufferCollector buffer) throws IOException {
			if (!valid()) {
				throw new IOException("JBufferRun not valid!");
			}
			buffer.spill(data, index, JBufferCollector.SpillOp.COPY);
			fresh = false;
		}
		
		/**
		 * Create a new snapshot.
		 * @param reader Input data to be sent to the new snapshot.
		 * @param length Length of the data.
		 * @param progress Progress of this snapshot.
		 * @throws IOException
		 */
		private void 
		snapshot(IFile.Reader<K, V> reader, long length, float progress) 
		throws IOException {
			if (this.progress < progress) {
				runs++;
				Path data = fileHandle.getInputSnapshotFileForWrite(task.getTaskID(), taskid, runs, length);
				Path index = fileHandle.getInputSnapshotIndexFileForWrite(task.getTaskID(), taskid, runs, 1096);
				FSDataOutputStream out  = localFs.create(data, false);
				FSDataOutputStream idx = localFs.create(index, false);
				if (out == null) throw new IOException("Unable to create snapshot " + data);
				write(reader, out, idx);
				this.length   = length;
				this.progress = progress;
				this.data     = data;
				this.index    = index;
				this.fresh    = true;
			}
		}
		
		private void write(IFile.Reader<K, V> in, FSDataOutputStream out, FSDataOutputStream index) throws IOException {
			DataInputBuffer key = new DataInputBuffer();
			DataInputBuffer value = new DataInputBuffer();
			IFile.Writer<K, V> writer = new IFile.Writer<K, V>(conf, out,  inputKeyClass, inputValClass, codec);
			/* Copy over the data until done. */
			while (in.next(key, value)) {
				writer.append(key, value);
			}
			writer.close();
			out.close();
			
			/* Write the index file. */
			index.writeLong(0);
			index.writeLong(writer.getRawLength());
			index.writeLong(out.getPos());

			/* Close everything. */
			index.flush();
			index.close();
		}
	}

	
	private JobConf conf;
	
	private FileSystem localFs;
	
	private Task task;
	
	private FileHandle fileHandle;
	
	private Class<K> inputKeyClass;
	
	private Class<V> inputValClass;
	
	private CompressionCodec codec;
	
	private Map<TaskID, Snapshot> snapshots;
	
	private float progress;
	
	public SnapshotManager(JobConf conf, Task task, 
			               Class<K> inputKeyClass, Class<V> inputValClass, 
			               Class<? extends CompressionCodec> inputCodecClass) 
	throws IOException {
		this.conf = conf;
		this.task = task;
		this.fileHandle = new FileHandle(task.getJobID());;
		this.fileHandle.setConf(conf);
		
		this.snapshots = new HashMap<TaskID, Snapshot>();
		this.localFs = FileSystem.getLocal(conf);
		this.progress = 0f;
		
		this.inputKeyClass = inputKeyClass;
		this.inputValClass = inputValClass;
		this.codec = inputCodecClass == null ? null :
			(CompressionCodec) ReflectionUtils.newInstance(inputCodecClass, conf);
	}

	
	public void collect(TaskID taskid, IFile.Reader<K, V> reader, long length, float progress) throws IOException {
		synchronized (this.snapshots) {
			if (!this.snapshots.containsKey(taskid)) {
				this.snapshots.put(taskid, new Snapshot(taskid));
			}
			
			Snapshot snapshot = this.snapshots.get(taskid);
			if (snapshot.progress < progress) {
				snapshot.snapshot(reader, length, progress);
			}
			
			this.progress = 0f;
			for (SnapshotManager.Snapshot s : this.snapshots.values()) {
				if (s.valid()) {
					this.progress += s.progress;
				}
			}
			this.progress /= (float) task.getNumberOfInputs();
		}
	}
	
	public float snapshot() throws IOException {
		List<SnapshotManager.Snapshot> snapshots = new ArrayList<SnapshotManager.Snapshot>();
		float progress = 0f;
		synchronized (this.snapshots) {
			for (SnapshotManager.Snapshot snapshot : this.snapshots.values()) {
				if (snapshot.valid()) {
					progress += snapshot.progress;
					snapshots.add(snapshot);
				}
			}
			progress = progress / (float) task.getNumberOfInputs();
		}

		LOG.debug("SnapshotThread: " + task.getTaskID() + " perform snapshot. progress " + progress);
		task.snapshots(snapshots, progress);
		return progress;
	}
	
	public float progress() {
		synchronized (this.snapshots) {
			return this.progress;
		}
	}
}
