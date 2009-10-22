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

public class SnapshotCollector<K extends Object, V extends Object> implements Runnable {
	private static final Log LOG = LogFactory.getLog(SnapshotCollector.class.getName());

	
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
			Class <K> keyClass = (Class<K>)conf.getInputKeyClass();
			Class <V> valClass = (Class<V>)conf.getInputValueClass();
			CompressionCodec codec = null;
			if (conf.getCompressMapOutput()) {
				Class<? extends CompressionCodec> codecClass =
					conf.getMapOutputCompressorClass(DefaultCodec.class);
				codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
			}

			DataInputBuffer key = new DataInputBuffer();
			DataInputBuffer value = new DataInputBuffer();
			IFile.Writer<K, V> writer = new IFile.Writer<K, V>(conf, out,  keyClass, valClass, codec);
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
	
	private boolean busy = false;
	private boolean open = true;
	private float lastSnapshotProgress = 0f;
	
	private Map<TaskID, Snapshot> snapshots;
	
	public SnapshotCollector(JobConf conf, Task task) throws IOException {
		this.conf = conf;
		this.task = task;
		this.fileHandle = new FileHandle(task.getJobID());;
		this.snapshots = new HashMap<TaskID, Snapshot>();
		this.localFs = FileSystem.get(conf);
	}

	
	public void close() throws IOException {
		if (!open) return;
		else open = false;
		
		synchronized (this) {
			this.notifyAll();
			while (busy) {
				try { this.wait();
				} catch (InterruptedException e) { }
			}
		}
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
		}
	}
	
	public float snapshot() throws IOException {
		List<SnapshotCollector.Snapshot> snapshots = new ArrayList<SnapshotCollector.Snapshot>();
		float progress = 0f;
		for (SnapshotCollector.Snapshot snapshot : this.snapshots.values()) {
			if (snapshot.valid()) {
				progress += snapshot.progress;
				snapshots.add(snapshot);
			}
		}
		progress = progress / (float) task.getNumberOfInputs();

		LOG.debug("SnapshotThread: " + task.getTaskID() + " perform snapshot. progress " + progress);
		task.snapshots(snapshots, progress);
		this.lastSnapshotProgress = progress;
		return progress;
	}
	
	public void run() {
		try {
			while (open) {
				synchronized (this) {
					busy = false;
					this.notifyAll();

					int freshRuns = 0;
					do {
						if (!open) return;
						try { this.wait();
						} catch (InterruptedException e) { return; }
						if (!open) return;

						freshRuns = 0;
						for (Snapshot snapshot : snapshots.values()) {
							if (snapshot.fresh) freshRuns++;
						}
					} while (freshRuns < snapshots.size() / 3);
					busy = true;
				}
				try {
					snapshot();
				} catch (IOException e) {
					LOG.info("Snapshot thread terminated. " + e);
					return; // done snapshoting 
				}
			}
		} finally {
			synchronized (this) {
				busy = false;
				this.notifyAll();
			}
		}
	}
}
