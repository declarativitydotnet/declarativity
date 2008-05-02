package p2.lang.plan;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class ObjectReference extends Reference {
	
	private Expression object;
	
	private Field field;
	
	public ObjectReference(Expression object, Field field) {
		super(field.isEnumConstant() ? Enum.class : field.getType(), 
			  object.toString() + "." + field.getName());
		this.object = object;
		this.field = field;
	}

	@Override
	public Set<Variable> variables() {
		return object.variables();
	}
	
	public Expression object() {
		return this;
	}
	
	public Field field() {
		return this.field;
	}
}
