package jol.types.operator;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import jol.core.Runtime;
import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.net.IP;
import jol.net.NetworkBuffer;
import jol.net.NetworkMessage;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.function.TupleFunction;
import jol.types.table.TableName;

/**
 * A RemoteBuffer takes the input tuples and inserts them
 * into the {@link NetworkBuffer}. The operator sends an
 * empty TupleSet to the output.
 */
public class RemoteBuffer extends Operator {

	/** The head predicate that identifies the location attribute. */
	private Predicate predicate;
	
	/** Indicates whether the tuples represent a delete operation. */
	private boolean deletion;
	
	/** An accessor to the location/address attribute value. */
	private TupleFunction<Object> addressAccessor;
	
	/** 
	 * Create a new RemoteBuffer operator object.
	 * @param context The runtime context.
	 * @param predicate The (head) predicate with the remote location attribute.
	 * @param deletion true if tuples represent a delete operation.
	 * @throws PlannerException 
	 */
	public RemoteBuffer(Runtime context, Predicate predicate, boolean deletion) 
	throws PlannerException {
		super(context, predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.deletion = deletion;
		
		for (jol.lang.plan.Expression arg : predicate) {
			if (arg instanceof Variable) {
				Variable var = (Variable) arg;
				if (var.loc()) {
					addressAccessor = arg.function(predicate.schema());
					break;
				}
			}
		}
	}
	
	@Override
	public String toString() {
		return "REMOTE_BUFFER[" + predicate + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		if (tuples.isEmpty()) return tuples;
		
		/* Group tuples by the address attribute: (address, tuple) */
		Map<String, TupleSet> groupByAddress = new HashMap<String, TupleSet>();
		for (Tuple tuple : tuples) {
			String key = (String) this.addressAccessor.evaluate(tuple);
			if (!groupByAddress.containsKey(key)) {
				groupByAddress.put(key, new TupleSet(this.predicate.name()));
			}
			groupByAddress.get(key).add(tuple);
		}
		
		for (String address : groupByAddress.keySet()) {
			String protocol = address.substring(0, address.indexOf(':'));
			String location = address.substring(address.indexOf(':') + 1);
	        Tuple remote = new Tuple("network", "send", new IP(location), 
			        		         new NetworkMessage(protocol, this.program, predicate.name(), 
			         		                      		new TupleSet(predicate.name()), 
			        		        		            new TupleSet(predicate.name())));
			
			NetworkMessage message = (NetworkMessage) remote.value(NetworkBuffer.Field.MESSAGE.ordinal());
			if (this.deletion) {
				message.deletions().addAll(groupByAddress.get(address));
			}
			else {
				message.insertions().addAll(groupByAddress.get(address));
			}
			
			TableName bufferName = context.network().buffer().name();
			try {
				context.schedule("network", bufferName, new TupleSet(bufferName, remote), null);
			} catch (UpdateException e) {
				e.printStackTrace();
			}
		}
		
		return new TupleSet(this.predicate.name());
	}

	public Predicate predicate() {
		return this.predicate;
	}

}
