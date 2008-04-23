package lang.ast;

import java.lang.reflect.Constructor;
import java.util.List;

public class NewClass extends Expression {
	
	private Constructor constructor;
	
	private List<Expression> arguments;
	
	public NewClass(Constructor constructor, List<Expression> arguments) {
		this.constructor = constructor;
		this.arguments = arguments;
	}

	@Override
	public Class type() {
		return constructor.getDeclaringClass();
	}

}
