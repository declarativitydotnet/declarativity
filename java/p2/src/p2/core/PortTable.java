package p2.core;

import java.util.HashSet;
import javax.security.auth.callback.Callback;

import types.basic.Tuple;
import types.basic.TupleSet;
import types.element.Element;
import types.element.Port;
import types.exception.ElementException;
import types.exception.UpdateException;
import types.table.Key;
import types.table.ObjectTable;
import types.table.Schema;
import types.table.Table;

public class PortTable extends ObjectTable {
	
	/* The primary key is (OwnerID, PortKey, Interface) */
	private static final Key PRIMARY_KEY = new Key(1,2,3);
	
	private static final Schema SCHEMA = 
		new Schema(new Schema.Entry("RuleID",    String.class),
				   new Schema.Entry("OwnerID",   String.class),
				   new Schema.Entry("PortKey",   String.class),
				   new Schema.Entry("Interface", String.class),
				   new Schema.Entry("Type",      String.class),
				   new Schema.Entry("DestID",    String.class),
				   new Schema.Entry("Object",    Element.class));
	
	public PortTable(Name name, Schema schema, Integer size, Number lifetime, Key key) {
		super(name, schema, key);
	}
	
	@Override
	public boolean remove(Tuple tuple) throws UpdateException {
		Plumber plumber = (Plumber) Catalog.table(Plumber.class.getName());
		Port port = (Port) tuple.value(schema("Object"));
		Element correspondent = tuple.value(schema("DestID")) != null ?
				                (Element)plumber.element((String)tuple.value(schema("DestID"))) : null;
        try {
        	if (correspondent != null) {
        		if (tuple.value(schema("Interface")).equals(Port.Interface.INPUT)) {
        			correspondent.input(port.portKey(), null);
        		}
        		else {
        			correspondent.output(port.portKey(), null);
        		}
        	}
        }
		catch (ElementException e) {
			throw new UpdateException(e.toString());
		}
		return super.remove(tuple);
	}

	@Override
	public Tuple insert(Tuple tuple) throws UpdateException { 
		Plumber plumber = (Plumber) Catalog.table(Plumber.class.getName());
		Port port = (Port) tuple.value(schema("Object"));
		Element owner = tuple.value(schema("OwnerID")) != null ?
				                (Element)plumber.element((String)tuple.value(schema("OwnerID"))) : null;
		Element correspondent = tuple.value(schema("DestID")) != null ?
				                (Element)plumber.element((String)tuple.value(schema("DestID"))) : null;
				                
	    if (owner == null) {
	    	throw new UpdateException("Must specify a valid OwnerID!");
	    }
	    else if (tuple.value(schema("PortKey")) == null) {
	    	throw new UpdateException("Must specify a valid PortKey!");
	    }
	    else if (port == null) {
			port = new Port((Port.Interface)tuple.value(schema("Interface")),
					        (Port.Type)tuple.value(schema("Type")), owner, 
					        (String)tuple.value(schema("PortKey")));
			tuple.value(schema("Object"), port);
		}
	    if (correspondent != null) {
	    	port.correspondent(correspondent);
	    	try {
	    		if (tuple.value(schema("Interface")).equals(Port.Interface.INPUT)) {
	    			correspondent.input(port.portKey(), port);
	    		}
	    		else {
	    			correspondent.output(port.portKey(), port);
	    		}
	    	} catch (ElementException e) {
	    		throw new UpdateException(e.toString());
	    	}
	    }
	    return super.insert(tuple);
	}
}