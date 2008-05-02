package p2.lang.plan;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class StaticReference extends Reference {
	
	private Field field;
	
	public StaticReference(Class type, Field field) {
		super(field.isEnumConstant() ? Enum.class : field.getType(), 
			  type.getCanonicalName() + "." + field.getName());
		this.field = field;
	}
	
	public Field field() {
		return this.field;
	}
}
