package stasis.jni;

import java.util.Iterator;

import jol.core.Runtime;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.StasisTable;
import jol.types.table.TableName;

public class LinearHashNTA extends StasisTable {

	public LinearHashNTA(Runtime context, TableName name, Key key,
			TypeList attributeTypes) {
		super(context, name, key, attributeTypes);
		// TODO Auto-generated constructor stub
	}

	@Override
	protected boolean add(byte[] keybytes, byte[] valbytes)
			throws UpdateException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public Integer cardinality() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	protected boolean remove(byte[] keybytes, byte[] valbytes)
			throws UpdateException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	protected byte[] lookup(byte[] keybytes) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	protected Iterator<byte[][]> tupleBytes() {
		// TODO Auto-generated method stub
		return null;
	}

	static {
	    System.loadLibrary("stasis");
    }

	private native long 
	stasisJniLinearHashNTACardinality(long page, long slot);
	
	private native byte[]
	stasisJniLinearHashNTAInsert(long page, long slot,
							     byte[] keybytes,
								 byte[] valbytes);
	
	private native byte[]
	stasisJniLinearHashNTAAdd(long page, long slot,
							  byte[] keybytes);
	
	private native byte[]
	stasisJniLinearHashNTAIterator(long page, long slot);
	
	private native byte[][]
	stasisJniLinearHashNTANext(long page, long slot, byte[] it);

}
