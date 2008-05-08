package p2.types.table;

import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;

public class EventTable extends Table {

	public EventTable(String name, TypeList attributeTypes) {
		super(name, 0, 0f, new Key(), attributeTypes);
	}

	protected Tuple insert(Tuple t) throws UpdateException {
		throw new UpdateException("Can't insert tuple in event table");
	}

	@Override
	protected boolean remove(Tuple t) throws UpdateException {
		throw new UpdateException("Can't remove tuple from event table");
	}

	@Override
	public boolean isEvent() {
		return true;
	}

}
