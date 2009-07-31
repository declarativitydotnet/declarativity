package org.apache.hadoop.bufmanager;

import java.io.ByteArrayOutputStream;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputCollector;

public class Record<K extends Object, V extends Object> implements OutputCollector<K, V>, Writable {
	public interface RecordIterator<K extends Object, V extends Object> extends Iterator<Record<K, V>> {
		public Record<K, V> peek();
		
		public Record<K, V> next();
		
		public boolean hasNext();
	}
	
	public final static Record NULL_RECORD = new Record();
	
	protected byte[] keyB = null;
	protected byte[] valB = null;
	
	public K key = null;
	public V value = null;
	
    public Record() {
    }
    
    public Record(K key, V value) {
    	this.key   = key;
    	this.value = value;
    }
    
    public boolean isNull() {
    	return this.key == null;
    }
    
    @Override
    public String toString() {
    	return this.key + " -> " + this.value;
    }
    
    @Override
    public int hashCode() {
    	return this.key != null ? 
    			this.key.hashCode() : "null".hashCode();
    }
    
	public int size() {
		return keyB != null && valB != null ?
				keyB.length + valB.length : 0;
	}
	
	public void unmarshall(JobConf conf) throws IOException {
		if (this.keyB == null || this.valB == null) {
			throw new IOException("No Key/Value bytes!");
		}
		else if (this.key != null && this.value != null) {
			return; // Already done.
		}
		
	    Class<K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
	    Class<V> valClass = (Class<V>)conf.getMapOutputValueClass();
		
		this.key   = (K) unmarshall(conf, this.keyB, keyClass);
		this.value = (V) unmarshall(conf, this.valB, valClass); 
	}
	
	protected Object unmarshall(JobConf conf, byte [] buf, Class type) throws IOException {
    	SerializationFactory serializationFactory = new SerializationFactory(conf);
	    Deserializer<K> typeDeserializer = serializationFactory.getDeserializer(type);
	    
		DataInputBuffer typeIB = new DataInputBuffer();
		typeIB.reset(buf, buf.length);
		typeDeserializer.open(typeIB);
		return typeDeserializer.deserialize(null);
	}
	
	public void marshall(JobConf conf) throws IOException {
		if (this.key == null || this.value == null) {
			throw new IOException("No key/value to marshall!");
		}
		else if (this.keyB != null && this.valB != null) {
			return; // Already done
		}
		
	    Class<K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
	    Class<V> valClass = (Class<V>)conf.getMapOutputValueClass();
		
		this.keyB = marshall(conf, keyClass, this.key);
		this.valB = marshall(conf, valClass, this.value);
	}
	
	protected byte[] marshall(JobConf conf, Class type, Object o) throws IOException {
    	SerializationFactory serializationFactory = new SerializationFactory(conf);
    	Serializer typeSerializer = serializationFactory.getSerializer(type);
		
		ByteArrayOutputStream buf = new ByteArrayOutputStream();
		typeSerializer.open(buf);
		typeSerializer.serialize(o);
		return buf.toByteArray();
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		int keySize = WritableUtils.readVInt(in);
		int valSize = WritableUtils.readVInt(in);
		
		if (keySize < 0) return; // Null record.
		
		this.keyB = new byte[keySize];
		this.valB = new byte[valSize];
		
		in.readFully(this.keyB);
		in.readFully(this.valB);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		if (this.key == null) {
			/* Null record. (used as a sentinal) */
			WritableUtils.writeVInt(out, -1);
			WritableUtils.writeVInt(out, -1);
			return;
		}
		else if (keyB == null) {
			throw new IOException("Key/Value pair not marshalled!");
		}
		
		WritableUtils.writeVInt(out, this.keyB.length);
		WritableUtils.writeVInt(out, this.valB.length);
		
		out.write(this.keyB);
		out.write(this.valB);
	}

	@Override
	public void collect(K key, V value) throws IOException {
		this.key = key;
		this.value = value;
	}
}
