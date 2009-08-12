package jol.types.basic;

import java.io.Serializable;
import java.util.Set;

public interface TupleSet extends Set<Tuple>, Comparable<TupleSet>, Serializable {

	public void refCount(boolean count);

	public TupleSet clone();

	public long id();
}
