package org.apache.hadoop.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapred.JobConf;

public class RecordGroup<K extends Object, V extends Object> extends Record<K, V> {
	
	public List<V> values;
	
	private List<byte[]> valBufs;
	
	public RecordGroup() {
		super(null, null);
		this.values  = new ArrayList<V>();
		this.valBufs = new ArrayList<byte[]>();
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(super.key);
		sb.append(" -> ");
		sb.append(values.toString());
		return sb.toString();
	}
	
	@Override
	public void collect(K key, V value) throws IOException {
		this.key = key;
		this.values.add(value);
	}
	
	public void unmarshall(JobConf conf) throws IOException {
		if (this.keyB == null || this.valBufs.size() == 0) {
			throw new IOException("RecordGroup: Nothing to unmarshall!");
		}
		if (this.key != null && this.values.size() > 0) {
			return; // Already done
		}
		
	    Class<K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
		this.key   = (K) unmarshall(conf, this.keyB, keyClass);
		
	    Class<V> valClass = (Class<V>)conf.getMapOutputValueClass();
		this.values.clear();
		for (byte[] buf : valBufs) {
			this.values.add((V) unmarshall(conf, buf, valClass)); 
		}
	}
	
	public void marshall(JobConf conf) throws IOException {
		if (this.key == null || this.values.size() == 0) {
			throw new IOException("RecordGroup: No records to marshall!");
		}
		
	    Class<K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
		this.keyB = marshall(conf, keyClass, this.key);
		
	    Class<V> valClass = (Class<V>)conf.getMapOutputValueClass();
	    for (V value : values) {
	    	byte[] valBuf = marshall(conf, valClass, value);
	    	this.valBufs.add(valBuf);
	    }
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		/* Read in the key */
		int keySize = WritableUtils.readVInt(in);
		this.keyB = new byte[keySize];
		in.readFully(this.keyB);
		
		/* Read in the values */
		int numVals = WritableUtils.readVInt(in);
		for (int i = 0; i < numVals; i++) {
			int valSize = WritableUtils.readVInt(in);
		    byte[] buf = new byte[valSize];
		    in.readFully(buf);
		    this.valBufs.add(buf);
		}
	}

	@Override
	public void write(DataOutput out) throws IOException {
		if (keyB == null) {
			throw new IOException("Key/Value pair not marshalled!");
		}
		
		/* Write the key */
		WritableUtils.writeVInt(out, this.keyB.length);
		out.write(this.keyB);
		
		/* Write values */
		WritableUtils.writeVInt(out, this.valBufs.size());
		for (byte[] buf : valBufs) {
			WritableUtils.writeVInt(out, buf.length);
			out.write(buf);
		}
	}
}
