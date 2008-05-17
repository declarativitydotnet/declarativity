package p2.types.basic;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;

import p2.lang.plan.DontCare;
import p2.lang.plan.Variable;
import p2.types.table.Key;


public class Schema {
	
	protected String name;
	
	protected Hashtable<String, Variable> variables;


	public Schema(String name, List<Variable> variables) {
		this.name = name;
		this.variables = new Hashtable<String, Variable>();
		for (Variable variable : variables) {
			this.variables.put(variable.name(), variable);
		}
	}
	
	public Schema(Schema schema) {
		this.name = schema.name;
		this.variables = (Hashtable) schema.variables.clone();
	}
	
	public Schema(String name) {
		this.name = name;
		this.variables = new Hashtable<String, Variable>();
	}
	
	public String name() {
		return this.name;
	}
	
	public int size() {
		return this.variables.size();
	}
	
	public void append(Variable variable) {
		variable.position(this.variables.size());
		if (!(variable instanceof DontCare)) {
			this.variables.put(variable.name(), variable);
		}
	}
	
	public boolean contains(Variable variable) {
		return this.variables.containsKey(variable.name());
	}
	
	@Override
	public String toString() {
		return name() + variables().toString();
	}
	
	
	public final List<Class> types() {
		List<Class> types = new ArrayList<Class>();
		for (int position = 0; position < this.variables.size(); position++) {
			for (Variable variable : this.variables.values()) {
				if (variable.position() == position) {
					types.add(variable.type());
					break;
				}
			}
		}
		return types;
	}
	
	public final List<Variable> variables() {
		List<Variable> variables = new ArrayList<Variable>();
		for (int position = 0; position < this.variables.size(); position++) {
			for (Variable variable : this.variables.values()) {
				if (variable.position() == position) {
					variables.add(variable.clone());
					break;
				}
			}
		}
		return variables;
	}
	
	public final Variable variable(String name) {
		return this.variables.get(name).clone();
	}
	
	public final Class type(String name) {
		return this.variables.get(name).type();
	}
	
	public final int position(String name) {
		return this.variables.containsKey(name) ?
				this.variables.get(name).position() : -1;
	}
	
	public final Schema join(String name, Schema inner) {
		Schema join = new Schema(name);
		for (Variable variable : this.variables()) {
			if (!(variable instanceof DontCare)) {
				join.append(variable);
			}
		}
		
		for (Variable variable : inner.variables()) {
			if (!(variable instanceof DontCare) && !join.contains(variable)) {
				join.append(variable);
			}
		}
		return join;
	}

}
