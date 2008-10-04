package p2.types.operator;

import java.util.Hashtable;
import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.net.IP;
import p2.net.NetworkMessage;
import p2.net.NetworkBuffer;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.function.TupleFunction;
import p2.types.table.TableName;

public class RemoteBuffer extends Operator {

	private Predicate predicate;
	
	private boolean deletion;
	
	private TupleFunction<Comparable> addressAccessor;
	
	public RemoteBuffer(Predicate predicate, boolean deletion) {
		super(predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.deletion = deletion;
		
		for (p2.lang.plan.Expression arg : predicate) {
			if (arg instanceof Variable) {
				Variable var = (Variable) arg;
				if (var.loc()) {
					addressAccessor = arg.function();
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
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		if (tuples.size() == 0) return tuples;
		
		/* Group tuples by the address attribute: (address, tuble) */
		Hashtable<String, TupleSet> groupByAddress = new Hashtable<String, TupleSet>();
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
			
			TableName bufferName = p2.net.Network.bufferName();
			try {
				p2.core.Runtime.runtime().schedule("network", bufferName, new TupleSet(bufferName, remote), new TupleSet(bufferName));
			} catch (UpdateException e) {
				e.printStackTrace();
			}
		}
		
		return new TupleSet(this.predicate.name());
	}

	@Override
	public Schema schema() {
		return this.predicate.schema();
	}

	@Override
	public Set<Variable> requires() {
		return this.predicate.requires();
	}
	
	public Predicate predicate() {
		return this.predicate;
	}

}
