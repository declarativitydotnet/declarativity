package org.apache.hadoop.bufmanager;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Executor;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.Decompressor;
import org.apache.hadoop.mapred.JobConf;

public class JBufferGroup<K extends Object, V extends Object> 
       extends JBuffer<K, V> {
	
    protected ServerSocketChannel channel = null;
    
    protected InetSocketAddress channelAddress = null;
	
    private Map<BufferID, BufferReceiver> receivers;
    
    private Set<BufferID> outstandingRequests;
    
	private boolean close;
    
	public JBufferGroup(BufferUmbilicalProtocol umbilical, Executor executor, JobConf job, BufferID bufid) throws IOException {
		super(umbilical, executor, job, bufid, BufferType.SORTED);
		this.receivers = new HashMap<BufferID, BufferReceiver>();
		this.outstandingRequests = new HashSet<BufferID>();

		try {
			String host = InetAddress.getLocalHost().getCanonicalHostName();
			InetSocketAddress address = new InetSocketAddress(host, 0); 
			this.channel = ServerSocketChannel.open();
			this.channel.socket().bind(address);
			this.channelAddress = new InetSocketAddress(host, this.channel.socket().getLocalPort());
		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
			throw e;
		}
	}
	
	@Override
	public Iterator<Record<K, V>> iterator() throws IOException {
		synchronized (this) {
			while (! closed) {
				try { this.wait();
				} catch (InterruptedException e) { e.printStackTrace(); }
			}
		}
		
		return new RecordGroupIterator(super.iterator(), this.job.getOutputKeyComparator());
	}
	
	@Override
	public void cancel(BufferID requestID) throws IOException {
		synchronized (this) {
			if (this.outstandingRequests.contains(requestID)) {
				this.outstandingRequests.remove(requestID);
			}
			
			if (this.receivers.containsKey(requestID)) {
				this.receivers.get(requestID).close();
			}
			this.notifyAll();
		}
	}
	
	@Override
	public void transfer(BufferID bufid, String source) throws IOException {
		synchronized (this) {
			if (!closed) {
				BufferRequest request = new BufferRequest(bufid, source, this.channelAddress);

				this.outstandingRequests.add(bufid);
				umbilical.request(request);
			}
		}
	}
	
	@Override
	public void done(BufferReceiver receiver) {
		synchronized (this) {
			this.receivers.remove(receiver.requestID());
			this.notifyAll();
		}
	}
	
	public void close() throws IOException {
		synchronized (this) {
			/* Don't close until all requests have been satisfied */
			while (this.outstandingRequests.size() > 0 || this.receivers.size() > 0) {
				try {
					this.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			
			if (this.channel != null) {
				this.channel.close();
				this.channel = null;
				this.channelAddress = null;
			}
			
			super.close();
			this.notifyAll();
		}
	}
	
	@Override
	public void run() {
		while (!closed) {
			try {
				SocketChannel conn  = this.channel.accept();
				DataInputStream input  = null;
				CompressionCodec codec = codec();
				if (codec != null) {
					Decompressor decompressor = CodecPool.getDecompressor(codec);
					decompressor.reset();
					input = new DataInputStream(codec.createInputStream(conn.socket().getInputStream(), decompressor));
				}
				else {
					input = new DataInputStream(conn.socket().getInputStream());
				}

				synchronized (this) {
					BufferID requestID = new BufferID();
					requestID.readFields(input);
					if (this.outstandingRequests.contains(requestID)) {
						this.outstandingRequests.remove(requestID);

						BufferReceiver receiver = new BufferReceiver(requestID, this, input);
						receivers.put(requestID, receiver);
						executor.execute(receiver);
					}
					else {
						input.close();
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	
	private static class RecordGroupIterator<K extends Object, V extends Object> 
	        implements Iterator<RecordGroup<K, V>> {
		private Iterator<Record<K, V>> sortedIterator;
		
		private Record<K, V> currentRecord;
		
		private RawComparator<K> comparator;
		
		public RecordGroupIterator(Iterator<Record<K, V>> sortedIterator, RawComparator<K> comparator) {
			this.sortedIterator = sortedIterator;
			this.comparator     = comparator;
			this.currentRecord  = this.sortedIterator.hasNext() ? 
					              this.sortedIterator.next() : null;
		}

		@Override
		public boolean hasNext() {
			return this.currentRecord != null;
		}

		@Override
		public RecordGroup<K, V> next() {
			try {
				if (this.currentRecord != null) {
					RecordGroup<K, V> group = new RecordGroup<K, V>();
					group.collect(currentRecord.key, currentRecord.value);
					while (this.sortedIterator.hasNext()) {
						Record<K, V> record = this.sortedIterator.next();
						if (this.comparator.compare(group.key, record.key) == 0) {
							group.collect(record.key, record.value);
						}
						else {
							this.currentRecord = record;
							return group;
						}
					}
					this.currentRecord = null; // We're all done.
					return group;
				}
			} catch (IOException e) {
				return null;
			}
			return null;
		}

		@Override
		public void remove() {
			/* unsupported */
		}
	}

}
