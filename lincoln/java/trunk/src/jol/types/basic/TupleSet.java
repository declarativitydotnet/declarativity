package jol.types.basic;

import java.io.Serializable;
import java.util.Collection;

public interface TupleSet extends Collection<Tuple>, Comparable<TupleSet>, Serializable {

	public void refCount(boolean count);

	public TupleSet clone();

	public long id();
}
