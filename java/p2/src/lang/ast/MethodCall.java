package lang.ast;

import java.lang.reflect.Method;
import java.util.List;

public class MethodCall extends Expression {
	
	private Expression object;
	
	private Method method;
	
	private List<Expression> arguments;
	
	public MethodCall(Expression object, Method method, List<Expression> arguments) {
		this.object = object;
		this.method = method;
		this.arguments = arguments;
	}

	@Override
	public Class type() {
		return method.getReturnType();
	}

}
