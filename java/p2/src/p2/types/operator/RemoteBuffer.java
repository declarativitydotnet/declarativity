package p2.types.operator;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Rule;
import p2.lang.plan.Variable;
import p2.net.NetworkBuffer;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.BadKeyException;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.function.TupleFunction;

public class RemoteBuffer extends Operator {

	private String protocol;
	
	private Predicate predicate;
	
	private boolean deletion;
	
	private TupleFunction<Comparable> addressAccessor;
	
	public RemoteBuffer(String protocol, Predicate predicate, boolean deletion) {
		super(predicate.program(), predicate.rule());
		this.protocol = protocol;
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
			Tuple remote = null; 
			try {
				TupleSet lookup = p2.net.Manager.buffer.primary().lookup(this.protocol, address, this.program);
				if (lookup != null && lookup.size() > 0) {
					remote = lookup.iterator().next();
				}
				else {
			        remote = new Tuple(this.protocol, address, this.program,
					                   new TupleSet(predicate.name()), new TupleSet(predicate.name()));
				}
			} catch (BadKeyException e) {
				e.printStackTrace();
				System.exit(0);
			}
			
			if (this.deletion) {
				TupleSet d = (TupleSet) remote.value(NetworkBuffer.Field.DELETION.ordinal());
				d.addAll(groupByAddress.get(address));
			}
			else {
				TupleSet i = (TupleSet) remote.value(NetworkBuffer.Field.INSERTION.ordinal());
				i.addAll(groupByAddress.get(address));
			}
			
			
			try {
				p2.net.Manager.buffer.force(remote);
			} catch (UpdateException e) {
				e.printStackTrace();
				System.exit(0);
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
