package jol.types.exception;

import java.lang.reflect.Type;

public class TypeException extends Exception {
	private static final long serialVersionUID = 1L;

	public TypeException(Type source, Type value) {
		super("Type error: source " + source + " value " + value);
	}
	
	public TypeException(String error, Type type) {
		super(error + ":  type = " + type);
	}
}
