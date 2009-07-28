package org.apache.hadoop.bufmanager;

import java.io.IOException;

import org.apache.hadoop.ipc.VersionedProtocol;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public interface BufferUmbilicalProtocol extends VersionedProtocol {
	long versionID = 0;

	/**
	 * Create output buffers. 
	 * @param tid The identifier of the task that owns this buffer
	 * @param partition The partition number to which this bucket refers.
	 * @param jobFile The job file associated with the task.
	 * @param type The type of buffer being requested.
	 * @return A buffer identifier that can be used to reference the buffer
	 */
	public BufferID create(TaskAttemptID tid, int partition, String jobFile, Buffer.BufferType type) throws IOException;
	
	/**
	 * Free the buffer... garbage collect.
	 * @param bufid The buffer to close.
	 * @throws IOException
	 */
	public void free(BufferID bufid) throws IOException;
	
	/**
	 * No further tuples can be added to this buffer 
	 * when closed. Buffer is not garbage collected.
	 * @param bufid
	 * @throws IOException
	 */
	public void close(BufferID bufid) throws IOException;
	
	/**
	 * Add a record to a given output buffer.
	 * @param bufid The buffer identifier
	 * @param record The record to append.
	 * @throws IOException
	 */
	public void add(BufferID bufid, Record record) throws IOException;
	
	
	/**
	 * Fetch an input from a map task for a specific input buffer.
	 * @param destination The receiving buffer identifier.
	 * @param tid The identifier of the task that owns the buffer
	 * @param source The source location
	 * @throws IOException
	 */
	public void fetch(BufferID destination, TaskAttemptID tid, String source) throws IOException;
	
	public void cancelFetch(BufferID destination, TaskAttemptID tid) throws IOException;
	
	/**
	 * Get the next record from the input buffer
	 * @param bufid The input buffer identifier
	 * @param taskid The id of the task requesting the next record
	 * @return The next record, grouped by key value.
	 * @throws IOException
	 */
	public Record getNextRecord(BufferID bufid, TaskAttemptID taskid) throws IOException;


}
