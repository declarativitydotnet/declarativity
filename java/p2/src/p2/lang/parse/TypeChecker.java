package p2.lang.parse;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import p2.lang.plan.Aggregate;
import p2.lang.plan.Alias;
import p2.lang.plan.ArrayIndex;
import p2.lang.plan.Assignment;
import p2.lang.plan.Event;
import p2.lang.plan.Expression;
import p2.lang.plan.Fact;
import p2.lang.plan.Location;
import p2.lang.plan.MethodCall;
import p2.lang.plan.NewClass;
import p2.lang.plan.Null;
import p2.lang.plan.Predicate;
import p2.lang.plan.Range;
import p2.lang.plan.Rule;
import p2.lang.plan.Selection;
import p2.lang.plan.Table;
import p2.lang.plan.Term;
import p2.lang.plan.Value;
import p2.lang.plan.Variable;
import p2.lang.plan.Watch;
import p2.types.basic.Tuple;
import p2.types.table.Key;
import p2.types.table.Schema;
import xtc.Constants;
import xtc.tree.GNode;
import xtc.tree.Node;
import xtc.tree.Visitor;
import xtc.util.Pair;
import xtc.util.SymbolTable;
import xtc.util.Runtime;

/**
 * A visitor to type check Overlog programs.
 */
public final class TypeChecker extends Visitor {
	private static final Map<String, Class> NAME_TO_BASETYPE = new HashMap<String, Class>();

	static {
		NAME_TO_BASETYPE.put("boolean", Boolean.class);
		NAME_TO_BASETYPE.put("byte", Byte.class);
		NAME_TO_BASETYPE.put("string", String.class);
		NAME_TO_BASETYPE.put("char", Character.class);
		NAME_TO_BASETYPE.put("double", Double.class);
		NAME_TO_BASETYPE.put("float", Float.class);
		NAME_TO_BASETYPE.put("int", Integer.class);
		NAME_TO_BASETYPE.put("long", Long.class);
		NAME_TO_BASETYPE.put("short", Short.class);
		NAME_TO_BASETYPE.put("void", Void.class);
	}

	public static Class primitive(String name) {
		return NAME_TO_BASETYPE.get(name);
	}

	/** The runtime. */
	protected Runtime runtime;

	/** The symbol table. */
	protected SymbolTable table;
	
	private Set<String> ruleNames;

	/** A static counter for temporary variable names */
	private static int tmpNameCount = 0;

	/**
	 * Create a new Overlog analyzer.
	 *
	 * @param runtime The runtime.
	 */
	public TypeChecker(Runtime runtime) {
		this.runtime = runtime;
	}
	
	public SymbolTable table() {
		return this.table;
	}
	
	public void prepare() {
		this.table = p2.types.table.Table.catalog().symbolTable();
		this.ruleNames = new HashSet<String>();
	}

	/**
	 * Analyze the specified translation unit.
	 *
	 * @param root The translation unit.
	 * @return The corresponding symbol table.
	 */
	public Node analyze(Node node) {
		dispatch(node);
		return node;
	}

	// =========================================================================

	/**
	 * Find the least upper bound of the two types.
	 * @param x the first type
	 * @param y the second type
	 */
	private Class lub(final Class x, final Class y) {
		if (x.getClass().isAssignableFrom(y)) {
			return x;
		} else if (y.getClass().isAssignableFrom(x)) {
			return y;
		} else {
			return lub(x.getClass().getSuperclass(), y);
		}
	}

	// =========================================================================

	private Expression ensureBooleanValue(Expression expr) {
		if (Void.class == expr.type()) {
			return null; // Can't do it for void expression type
		}
		else if (Number.class.isAssignableFrom(expr.type())) {
			/* expr != 0 */
			return new p2.lang.plan.Boolean(p2.lang.plan.Boolean.NEQUAL,
					                    expr, new Value<Number>(0));
		}
		else if (!Boolean.class.isAssignableFrom(expr.type())) {
			/* expr != null*/
			return new p2.lang.plan.Boolean(p2.lang.plan.Boolean.NEQUAL,
									    expr, Null.NULLV);
		}
		return expr;
	}
	/**
	 * Visit all nodes in the AST.
	 */
	public void visit(final GNode n) {
		for (Object o : n) {
			if (o instanceof Node) {
				dispatch((Node) o);
			} else if (Node.isList(o)) {
				iterate(Node.toList(o));
			}
		}
	}
	
	public Class visitFact(final GNode n) {
		Class type = (Class) dispatch((Node)n.getNode(0));
		assert(type == String.class);
		String name = (String) n.getNode(0).getProperty(Constants.TYPE);
		
		List<Value> args = new ArrayList<Value>();
		if (n.size() != 0) {
			for (Node arg : n.<Node> getList(1)) {
				Class t = (Class) dispatch(arg);
				assert(Value.class.isAssignableFrom(t));
				args.add((Value)arg.getProperty(Constants.TYPE));
			}
		}
		
		Fact fact = new Fact(name, args);
		fact.location(n.getLocation());
		n.setProperty(Constants.TYPE, fact);
		return Fact.class;
	}
	
	public Class visitWatch(final GNode n) {
		Class type = (Class) dispatch((Node)n.getNode(0));
		assert(type == Value.class);
		Value<String> name = (Value<String>) n.getNode(0).getProperty(Constants.TYPE);
		
		if (!table.isDefined(name.value())) {
			runtime.error("Watch statement refers to unknown table/event name " + name.value());
			return Error.class;
		}
		String modifier = n.size() > 1 ? n.getString(1) : "";
		
		Watch watch = new Watch(name.value(), modifier);
		watch.location(n.getLocation());
		n.setProperty(Constants.TYPE, watch);
		return Watch.class;
	}
	
	public Class visitTable(final GNode n) {
		Class type;

		/************** Table Name ******************/
		type = (Class) dispatch(n.getNode(0));
		if (n.getNode(0).size() > 1) {
			runtime.error("Dot specificed names not allowd in definition statement! " + n.getNode(0));
			return Error.class;
		}
		assert(type == Value.class);
		Value<String> name = (Value<String>) n.getNode(0).getProperty(Constants.TYPE);

		/************** Table Size ******************/
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) {
			return Error.class;
		}
		
		assert(type == Value.class);
		Value size = (Value) n.getNode(1).getProperty(Constants.TYPE);
		if (!Integer.class.isAssignableFrom(size.type())) {
			runtime.error("Table size type is " + type.getClass() + " must be type " + Integer.class);
			return Error.class;
		}

		/************** Table Lifetime ******************/
		type = (Class) dispatch(n.getNode(2));
		if (type == Error.class) {
			return Error.class;
		}
	    assert(type == Value.class);	
	    
		Value lifetime = (Value) n.getNode(2).getProperty(Constants.TYPE);
		if (!Number.class.isAssignableFrom(lifetime.type())) {
			runtime.error("Lifetime type " + type + 
					" not allowed! Must be subtype of " + Number.class);
			return Error.class;
		}

		/************** Table Primary Key ******************/
		type = (Class) dispatch(n.getNode(3));
		if (type == Error.class) return type;
		assert (type == Key.class);
		Key key  = (Key) n.getNode(3).getProperty(Constants.TYPE);

		/************** Table Schema ******************/
		type = (Class) dispatch(n.getNode(4));
		if (type == Error.class) return type;
		assert(type == Schema.class);
		Schema schema  = (Schema) n.getNode(4).getProperty(Constants.TYPE);

		table.current().define(name.value(), schema.types());
		
		Table create = new Table(name, size, lifetime, key, schema);
		create.location(n.getLocation());
		n.setProperty(Constants.TYPE, create);
		return Table.class;
	}

	
	public Class visitEvent(final GNode n) {
		Class type;

		/************** Event Name ******************/
		type = (Class) dispatch(n.getNode(0));
		if (n.getNode(0).size() > 1) {
			runtime.error("Dot specificed names not allowed in definition statement! " + n.getNode(0));
			return Error.class;
		}
		Value<String> name = (Value<String>) n.getNode(0).getProperty(Constants.TYPE);
		
		/************** Table Schema ******************/
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return type;
		assert(type == Schema.class);
		Schema schema  = (Schema) n.getNode(1).getProperty(Constants.TYPE);

		table.current().define(name.value(), schema.types());
		
		Event event = new Event(name, schema);
		event.location(n.getLocation());
		n.setProperty(Constants.TYPE, event);
		return Event.class;
	}
	
	public Class visitKeys(final GNode n) {
		Integer[] keys = new Integer[n.size()];
		int index = 0;
		if (n.size() > 0) {
			for (Node k : n.<Node>getList(0)) {
				Class type = (Class) dispatch(k);
				if (type == Error.class) return Error.class;
				assert(type == Value.class);
				
				Value key  = (Value) k.getProperty(Constants.TYPE);
				if (!Integer.class.isAssignableFrom(key.type())) {
					runtime.error("Key must be of type Integer! type = " + key);
					return Error.class;
				}
				keys[index++] = ((Value<Integer>) key).value();
			}
		}
		n.setProperty(Constants.TYPE, new Key(keys));
		return Key.class;
	}
	
	public Class visitSchema(final GNode n) {
		List<Class> types = new ArrayList<Class>();
		for (Object t : n.getList(0)) {
			Class type = (Class) dispatch((Node)t);
			if (type == Error.class) return Error.class;
			types.add(type);
		}
		n.setProperty(Constants.TYPE, new Schema(types));
		return Schema.class;
	}
	
	public Class visitRule(final GNode n) {
		if (n.getString(0) == null) {
			runtime.error("All rules must define a rule name!");
			return Error.class;
		}
		String name = n.getString(0);
		
		if (ruleNames.contains(name)) {
			runtime.error("Multiple rule names defined as " 
					+ name + ". Rule name must be unique!");
			return Error.class;
		}
		Boolean deletion = n.getString(1) != null;
		
		
		Predicate head = null;
		List<Term> body = null;
		
		/* Rules define a scope for the variables defined. 
		 * The body has the highest scope followed by the
		 * head. */
		table.enter("Body:" + name);
		try {
			/* Evaluate the body first. */
			Class type = (Class) dispatch(n.getNode(3));
			
			if (type == Error.class) {
				return Error.class;
			}
			assert(type == List.class);
			body = (List<Term>) n.getNode(3).getProperty(Constants.TYPE);
				

			table.enter("Head:" + name);
			try {
				/* Evaluate the head. */
				type = (Class) dispatch(n.getNode(2));
				if (type == Error.class) {
					return Error.class;
				}
				assert (type == Predicate.class);
				head = (Predicate) n.getNode(2).getProperty(Constants.TYPE);
				if (!deletion) {
					for (Expression arg : head) {
						if (arg instanceof Variable) {
							Variable var = (Variable) arg;
							if (var.isDontCare()) {
								runtime.error("Head predicate in a non-deletion rule " +
										      "can't contain don't care variable!", n);
								return Error.class;
							}
						}
					}
				}
			} finally {
				table.exit();
			}
		} finally {
			table.exit();
		}
		
		Rule rule = new Rule(name, deletion, head, body);
		rule.location(n.getLocation());
		n.setProperty(Constants.TYPE, rule);
		return Rule.class;
	}

	public Class visitRuleHead(final GNode n) {
		/* Visit the tuple. */
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		assert(type == Predicate.class);
		Predicate head = (Predicate) n.getNode(0).getProperty(Constants.TYPE);
		
		if (head.notin()) {
			runtime.error("Can't apply notin to head predicate!", n);
			return Error.class;
		}
		else if (head.event() != null) {
			runtime.error("Can't apply event modifier to rule head!", n);
			return Error.class;
		}

		/* All variables mentioned in the head must be in the body. 
		 * The types must also match. */
		for (Expression argument : head) {
			if (argument instanceof Variable) {
				Variable var = (Variable) argument;
				if (!var.isDontCare()) {
					Class headType = (Class) table.current().lookupLocally(var.name());
					Class bodyType = (Class) table.current().getParent().lookupLocally(var.name());
					if (bodyType == null) {
						runtime.error("Head variable " + var
								+ " not defined in rule body.");
						return Error.class;
					} else if (!headType.isAssignableFrom(bodyType)) {
						runtime.error("Type mismatch: Head variable " + var
								+ " type is " + headType + ", but defined as type "
								+ bodyType + " in rule body.");
						return Error.class;
					}
				}
			}
		}

		n.setProperty(Constants.TYPE, head);
		return Predicate.class;
	}

	public Class visitRuleBody(final GNode n) {
		List<Term> terms = new ArrayList<Term>();
		
		for (Node term : n.<Node> getList(0)) {
			Class type = (Class) dispatch(term);
			if (type == Error.class) {
				return type;
			}
			assert(Term.class.isAssignableFrom(type));
			Term t = (Term) term.getProperty(Constants.TYPE);
			if (t instanceof Predicate) {
				Predicate p = (Predicate) t;
				if (p.event() != null && p.notin()) {
					runtime.error("Can't apply notin to event predicate!",n);
					return Error.class;
				}
			}
			terms.add(t);
		}

		n.setProperty(Constants.TYPE, terms);
		return List.class;
	}

	public Class visitPredicate(final GNode n) {
		boolean notin = n.getString(0) != null;
		String event = n.getString(2);
		
		Class type = (Class) dispatch(n.getNode(1));
		assert(type == Value.class);
		Value<String> name = (Value<String>) n.getNode(1).getProperty(Constants.TYPE);
		
		/* Lookup the schema for the given tuple name. */
		List<Class> schema = (List<Class>) table.lookup(name.value());
		if (schema == null) {
			runtime.error("No schema defined for predicate " + name);
			return Error.class;
		}
		
		type = (Class) dispatch(n.getNode(3));
		assert(type == List.class);
		List<Expression> parameters = (List<Expression>) n.getNode(3).getProperty(Constants.TYPE);
		List<Expression> arguments = new ArrayList<Expression>();
		/* Type check each tuple argument according to the schema. */
		for (int index = 0; index < schema.size(); index++) {
			Expression param = parameters.size() <= index ? 
					           Variable.dontCare(schema.get(index)) : parameters.get(index);
			if (Alias.class.isAssignableFrom(param.getClass())) {
				Alias alias = (Alias) param;
				if (alias.field() < index) {
					runtime.error("Alias fields must be in numeric order!");
					return Error.class;
				}
				/* Fill in missing variables with tmp variables. */
				while (index < alias.field()) {
					arguments.add(Variable.dontCare(schema.get(index++)));
				}
			}
			
			if (Variable.class.isAssignableFrom(param.getClass())) {
				/* Only look in the current scope. */
				Variable var = (Variable) param;
				if (var.type() == null) {
					/* Fill in type using the schema. */
					var.type(schema.get(index));
				}
				
				/* Map variable to its type. */
				table.current().define(var.name(), var.type());
				
			}

			/* Ensure the type matches the schema definition. */
			if (!schema.get(index).isAssignableFrom(param.type())) {
				runtime.error("Predicate " + name.value() + " argument " + index
						+ " type " + param.type() + " does not match type " 
						+ schema.get(index) + " in schema.", n);
				return Error.class;
			}
			
			arguments.add(param);
		}

		Predicate pred = new Predicate(notin, name.value(), event, arguments);
		pred.location(n.getLocation());
		n.setProperty(Constants.TYPE, pred);
		return Predicate.class;
	}

	public Class visitTupleName(final GNode n) {
		String name = n.size() > 1 ? n.getString(0) + "." + n.getString(1)
				                   : n.getString(0);
		n.setProperty(Constants.TYPE, new Value<String>(name));
		return Value.class;
	}

	public Class visitAssignment(final GNode n) {
		/* Variable. */
		Class type = (Class) dispatch(n.getNode(0));
		if (!(type == Variable.class || type == Location.class)) {
			runtime.error("Cannot assign to type " + type + 
					" must be of type Variable or Location variable!");
			return Error.class;
		}
		Variable var = (Variable) n.getNode(0).getProperty(Constants.TYPE);

		type = (Class) dispatch(n.getNode(1));
		assert(Expression.class.isAssignableFrom(type));
		Expression expr = (Expression) n.getNode(1).getProperty(Constants.TYPE);

		if (var.type() == null) {
			var.type(expr.type());
			table.current().define(var.name(), expr.type());
		}
		else if (!var.type().isAssignableFrom(expr.type())) {
			runtime.error("Assignment type mismatch: variable " + var.name() + 
					" of type " + var.type() + 
					" cannot be assigned a value of type " + expr.type());
			return Error.class;
		}

		Assignment assign = new Assignment(var, expr);
		assign.location(n.getLocation());
		n.setProperty(Constants.TYPE, assign);
		return Assignment.class;
	}
	
	public Class visitSelection(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		assert (Expression.class.isAssignableFrom(type));
		Expression expr = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		
		Selection select = new Selection(ensureBooleanValue(expr));
		select.location(n.getLocation());
		n.setProperty(Constants.TYPE, select);
		return Selection.class;
	}

	//---------------------------- Expressions -------------------------------//
	public Class visitExpression(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		Expression expr = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		expr.location(n.getLocation()); // Set the location of this expression.
		n.setProperty(Constants.TYPE, n.getNode(0).getProperty(Constants.TYPE));
		return type;
	}

	public Class visitLogicalOrExpression(final GNode n) {
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(2));
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);

		if (ensureBooleanValue(lhs) == null) {
			runtime.error("Cannot evaluate type " + lhs.type()
					+ " in a logical or expression", n);
			return Error.class;
		} else if (ensureBooleanValue(rhs) == null) {
			runtime.error("Cannot evaluate void type " + rhs.type()
					+ " in a logical or expression", n);
			return Error.class;
		}
		n.setProperty(Constants.TYPE, 
			new p2.lang.plan.Boolean(p2.lang.plan.Boolean.OR, 
					ensureBooleanValue(lhs), 
					ensureBooleanValue(rhs)));
		return p2.lang.plan.Boolean.class;
	}

	public Class visitLogicalAndExpression(final GNode n) {
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(2));
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);

		if (ensureBooleanValue(lhs) == null) {
			runtime.error("Cannot evaluate type " + lhs.type()
					+ " in a logical and expression", n);
			return Error.class;
		} else if (ensureBooleanValue(rhs) == null) {
			runtime.error("Cannot evaluate void type " + rhs.type()
					+ " in a logical and expression", n);
			return Error.class;
		}
		n.setProperty(Constants.TYPE, 
			new p2.lang.plan.Boolean(p2.lang.plan.Boolean.AND, 
					ensureBooleanValue(lhs), 
					ensureBooleanValue(rhs)));
		return p2.lang.plan.Boolean.class;
	}

	public Class visitEqualityExpression(final GNode n) {
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(2));
		if (ltype == Error.class || rtype == Error.class) {
			return Error.class;
		}
		
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		String oper = n.getString(1);
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);

		if (Void.class == lhs.type()) {
			runtime.error("Cannot evaluate type " + lhs.type()
					+ " in a logical and expression", n);
			return Error.class;
		} else if (Void.class == rhs.type()) {
			runtime.error("Cannot evaluate void type " + rhs.type()
					+ " in a logical and expression", n);
			return Error.class;
		}
		n.setProperty(Constants.TYPE, 
			new p2.lang.plan.Boolean(oper,  lhs,  rhs));
		return p2.lang.plan.Boolean.class;
	}
	
	public Class visitLogicalNegationExpression(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
			
		Expression expr = (Expression) n.getNode(0).getProperty(Constants.TYPE);

		if (ensureBooleanValue(expr) == null) {
			runtime.error("Type error: cannot evaluate !" + expr.type(), n);
			return Error.class;
		} 
		n.setProperty(Constants.TYPE, 
				new p2.lang.plan.Boolean(p2.lang.plan.Boolean.NOT, ensureBooleanValue(expr), null));
		return p2.lang.plan.Boolean.class;
	}

	public Class visitInclusiveExpression(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		
		if (type == Error.class) {
			return Error.class;
		}
		else if (!Variable.class.isAssignableFrom(type)) {
			runtime.error("Type error: left hand side of IN operator must be a Variable!", n);
			return Error.class;
		}
		Variable variable = (Variable) n.getNode(0).getProperty(Constants.TYPE);
		
		type = (Class) dispatch(n.getNode(2));
		if (type == Error.class) { 
			return Error.class;
		}
		
		Expression expr = (Expression) n.getNode(2).getProperty(Constants.TYPE);
		if (Range.class.isAssignableFrom(type) || Collection.class.isAssignableFrom(expr.type())) {
			n.setProperty(Constants.TYPE, new p2.lang.plan.Boolean(p2.lang.plan.Boolean.IN, variable, expr));
			return p2.lang.plan.Boolean.class;
		}

		runtime.error("Type error: right hand side of IN operator must be " +
				      "a range expression or implement " + Collection.class, n);
		return Error.class;
	}

	public Class visitRangeExpression(final GNode n) {
		Class type = (Class) dispatch(n.getNode(1));
		assert(Expression.class.isAssignableFrom(type));
		type = (Class) dispatch(n.getNode(2));
		assert(Expression.class.isAssignableFrom(type));
		
		Expression begin = (Expression) n.getNode(1).getProperty(Constants.TYPE);
		Expression end   = (Expression) n.getNode(2).getProperty(Constants.TYPE);
		
		if (begin.type() != end.type()) {
			runtime.error("Type error: range begin type " + begin.type() + 
					      " != range end type" + end.type());
			return Error.class;
		}
		else if (!Number.class.isAssignableFrom(begin.type())) {
			runtime.error("Type error: range boundaries must be subtype of "  + 
					       Number.class, n);
			return Error.class;
		}
		
		String marker  = n.getString(0) + n.getString(3);
		if ("[]".equals(marker)) {
			n.setProperty(Constants.TYPE, new Range(Range.Operator.CC, begin, end));
		}
		else if ("(]".equals(marker)) {
			n.setProperty(Constants.TYPE, new Range(Range.Operator.OC, begin, end));
		}
		else if ("[)".equals(marker)) {
			n.setProperty(Constants.TYPE, new Range(Range.Operator.CO, begin, end));
		}
		else if ("()".equals(marker)) {
			n.setProperty(Constants.TYPE, new Range(Range.Operator.OO, begin, end));
		}
		else {
			assert(false);
		}
		
		return Range.class;
	}
	
	//----------------------- Math Expressions --------------------------//

	public Class visitShiftExpression(final GNode n) {
		String oper = n.getString(1);
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(2));
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);
		
		if (lhs.type() == Integer.class && rhs.type() == Integer.class) {
			n.setProperty(Constants.TYPE, new p2.lang.plan.Math(oper, lhs, rhs));
			return p2.lang.plan.Math.class;
		} else {
			runtime.error("Cannot shift type " + lhs.type() + 
					" using type " + rhs.type(), n);
			return Error.class;
		}
	}

	public Class visitAdditiveExpression(final GNode n) {
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(2));
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		String oper = n.getString(1);
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);

		if (Number.class.isAssignableFrom(lhs.type()) && 
			Number.class.isAssignableFrom(rhs.type())) {
			n.setProperty(Constants.TYPE, new p2.lang.plan.Math(oper, lhs, rhs));
			return p2.lang.plan.Math.class;
		}
		runtime.error("Type mismatch: " + lhs.type() + 
				" " + oper + " " + rhs.type(), n);
		return Error.class;
	}

	public Class visitMultiplicativeExpression(final GNode n) {
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(2));
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		String oper = n.getString(1);
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);

		if (Number.class.isAssignableFrom(lhs.type()) && 
			Number.class.isAssignableFrom(rhs.type())) {
			n.setProperty(Constants.TYPE, new p2.lang.plan.Math(oper, lhs, rhs));
			return p2.lang.plan.Math.class;
		}
		runtime.error("Type mismatch: " + lhs.type() + 
				" " + oper + " " + rhs.type(), n);
		return Error.class;
	}

	//---------------------------- Postfix Expressions ------------------------//

	public Class visitMethod(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0)); 
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
		Expression object = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		
		String name = n.getString(1);
		System.err.println("Method location " + n.getLocation());
		
		type = (Class) dispatch(n.getNode(2)); 
		assert(type == List.class);
		List<Expression> arguments = 
			(List<Expression>) n.getNode(2).getProperty(Constants.TYPE);
		List<Class> parameterTypes = new ArrayList<Class>();
		for (Expression arg : arguments) {
			parameterTypes.add(arg.type());
		}
		
		try {
			Class [] types = parameterTypes.toArray(new Class[parameterTypes.size()]);
			Method method = object.type().getDeclaredMethod(name, types);
			n.setProperty(Constants.TYPE, new MethodCall(object, method, arguments));
		} catch (SecurityException e) {
			runtime.error("Method error: " + e.toString(), n);
			return Error.class;
		} catch (NoSuchMethodException e) {
			runtime.error("Method error: " + e.toString(), n);
			return Error.class;
		}
		
		return MethodCall.class;
	}
	
	public Class visitNewClass(final GNode n) {
		Class type = (Class) dispatch(n.getNode(1));
		assert(type == List.class);
		
		List<Expression> arguments = (List<Expression>) n.getNode(1).getProperty(Constants.TYPE);
		List<Class> parameterTypes = new ArrayList<Class>();
		for (Expression arg : arguments) {
			parameterTypes.add(arg.type());
		}
		Class [] types = (Class[]) parameterTypes.toArray();
		
		try {
			type = (Class) dispatch(n.getNode(0));
			Constructor constructor = type.getConstructor(types);
			n.setProperty(Constants.TYPE, new NewClass(constructor, arguments));
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NoSuchMethodException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return NewClass.class;
	}
	
	public Class visitArrayIndex(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0)); 
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
		
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return Error.class;
		assert(Value.class == type);
		
		Expression    object = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Value<Integer> index = (Value<Integer>) n.getNode(1).getProperty(Constants.TYPE);
		
		if (object.type() == null) {
			runtime.error("Type error: " + object.toString() + " unknown type!");
			return Error.class;
		}
		else if (!object.type().isArray()) {
			runtime.error("Type error: " + object.toString() + " is not an array type!");
			return Error.class;
		}
		n.setProperty(Constants.TYPE, new ArrayIndex(object, index.value()));
		return ArrayIndex.class;
	}
	
	public Class visitIncrement(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0)); 
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
		
		Expression expr = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		if (expr.type() == null) {
			runtime.error("Unable to resolve expression type " + expr); 
			return Error.class;
		}
		else if  (!Number.class.isAssignableFrom(expr.type())) {
			runtime.error("Expression " + expr + 
					" type must be numberic in increment expression.");
		}
		n.setProperty(Constants.TYPE, new p2.lang.plan.Math(p2.lang.plan.Math.INC, expr, null));
		return p2.lang.plan.Math.class;
	}
	
	public Class visitDecrement(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0)); 
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
		
		Expression expr = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		if (expr.type() == null) {
			runtime.error("Unable to resolve expression type " + expr); 
			return Error.class;
		}
		else if  (!Number.class.isAssignableFrom(expr.type())) {
			runtime.error("Expression " + expr + 
					" type must be numberic in decrement expression.");
		}
		n.setProperty(Constants.TYPE, new p2.lang.plan.Math(p2.lang.plan.Math.DEC, expr, null));
		return p2.lang.plan.Math.class;
	}
	
	
	//---------------------------- Arguments ------------------------//
	public Class visitArguments(final GNode n) {
		List<Expression> args = new ArrayList<Expression>();
		if (n.size() != 0) {
			for (Node arg : n.<Node> getList(0)) {
				Class t = (Class) dispatch(arg);
				assert(Expression.class.isAssignableFrom(t));
				args.add((Expression)arg.getProperty(Constants.TYPE));
			}
		}
		n.setProperty(Constants.TYPE, args);
		return List.class;
	}
	
	
	//---------------------------- Identifiers ------------------------//
	/***********************************************************
	 * Definitions from Identifier.rats
	 ***********************************************************/

	public Class visitType(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		if (n.getNode(1) != null) {
			dispatch(n.getNode(1));
			int[] dim = (int[]) n.getNode(1).getProperty(Constants.TYPE);
			Object instance = Array.newInstance(type, dim);
			type = instance.getClass();
			System.err.println("Class: " + type.getName() );
		}
		n.setProperty(Constants.TYPE, type);
		return type;
	}
	
	public Class visitDimensions(final GNode n) {
		int[] dims = new int[n.size()];
		n.setProperty(Constants.TYPE, dims);
		return int[].class;
	}

	public Class visitClassType(final GNode n) {
		try {
			String name = n.getString(0);
			for (Object c : n.getList(1)) {
				name  += "." + c.toString();
			}
			Class type = Class.forName(name);
			return type;
		} catch (ClassNotFoundException e) {
			runtime.error("Bad class type: " + n.getString(0));
		}
		return Error.class;
	}

	public Class visitPrimitiveType(final GNode n) {
		return primitive(n.getString(0));
	}

	public Class visitVariable(final GNode n) {
		Class type =  (Class)  table.current().lookup(n.getString(0));
		if (Variable.DONTCARE.equals(n.getString(0))) {
			/* Generate a fake variable for all don't cares. */
			n.setProperty(Constants.TYPE, Variable.dontCare(null));
		}
		else {
			n.setProperty(Constants.TYPE, new Variable(n.getString(0), type));
		}
		return Variable.class;
	}

	public Class visitLocation(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		assert (type == Variable.class);
		Variable var = (Variable) n.getNode(0).getProperty(Constants.TYPE);
		n.setProperty(Constants.TYPE, new Location(var.name(), var.type()));
		return Location.class;
	}

	public Class visitAggregate(final GNode n) {
		if (n.getNode(1) == null) {
			n.setProperty(Constants.TYPE, new Aggregate(null, n.getString(0), null));
		}
		else {
			Class type = (Class) dispatch(n.getNode(1));
			assert (type == Variable.class);
			Variable var = (Variable) n.getNode(1).getProperty(Constants.TYPE);
			n.setProperty(Constants.TYPE, new Aggregate(var.name(), n.getString(0), var.type()));
		}
		return Aggregate.class;
	}
	
	public Class visitAlias(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		assert (type == Variable.class);
		Variable var = (Variable) n.getNode(0).getProperty(Constants.TYPE);
		
		type = (Class) dispatch(n.getNode(1));
		assert (type == Value.class);
		Value<Integer> val = (Value<Integer>) n.getNode(1).getProperty(Constants.TYPE);
		
		n.setProperty(Constants.TYPE,  new Alias(var.name(), val.value(), var.type()));
		return Alias.class;
	}

	/***********************************************************
	 * Definitions from Constant.rats
	 ***********************************************************/

	public Class visitFloatConstant(final GNode n) {
		n.setProperty(Constants.TYPE, new Value<Float>(Float.parseFloat(n.getString(0))));
		return Value.class;
	}

	public Class visitIntegerConstant(final GNode n) {
		n.setProperty(Constants.TYPE, new Value<Integer>(Integer.parseInt(n.getString(0))));
		return Value.class;
	}

	public Class visitStringConstant(final GNode n) {
		n.setProperty(Constants.TYPE, new Value<String>(n.getString(0)));
		return Value.class;
	}

	public Class visitBooleanConstant(final GNode n) {
		if (n.getString(0).equals("true")) {
			n.setProperty(Constants.TYPE, new Value<Boolean>(Boolean.TRUE));
		}
		else {
			n.setProperty(Constants.TYPE, new Value<Boolean>(Boolean.FALSE));
		}
		return Value.class;
	}

	public Class visitNullConstant(final GNode n) {
		n.setProperty(Constants.TYPE, Null.NULLV);
		return Value.class;
	}

	public Class visitInfinityConstant(final GNode n) {
		n.setProperty(Constants.TYPE, new Value<Integer>(Integer.MAX_VALUE));
		return Value.class;
	}
	
	public Class visitFloatMatrix(final GNode n) {
		List<Node> elements = n.<Node>getList(0).list();
		Float[][] matrix = null;
		int index = 0;
		for (Node v : elements) {
			Class type = (Class) dispatch(v);
			assert(type == Value.class);
			Value<Float[]> val = (Value<Float[]>) v.getProperty(Constants.TYPE);
			Float[] vector = val.value();
			
			if (matrix == null) {
				matrix = new Float[elements.size()][vector.length];
			}
			else if (matrix[index-1].length != vector.length) {
				runtime.error("Matrix vector lengths mismatch!");
				return Error.class;
			}
			matrix[index++] = vector;
		}
		n.setProperty(Constants.TYPE, new Value<Float[][]>(matrix));
		return Value.class;
	}

	public Class visitIntMatrix(final GNode n) {
		List<Node> elements = n.<Node>getList(0).list();
		Integer[][] matrix = null;
		int index = 0;
		for (Node v : elements) {
			Class type = (Class) dispatch(v);
			assert(type == Value.class);
			Value<Integer[]> val = (Value<Integer[]>) v.getProperty(Constants.TYPE);
			Integer[] vector = val.value();
			
			if (matrix == null) {
				matrix = new Integer[elements.size()][vector.length];
			}
			else if (matrix[index-1].length != vector.length) {
				runtime.error("Matrix vector lengths mismatch!");
				return Error.class;
			}
			matrix[index++] = vector;
		}
		n.setProperty(Constants.TYPE, new Value<Integer[][]>(matrix));
		return Value.class;
	}

	public Class visitIntVector(final GNode n) {
		List<Node> elements = n.<Node>getList(0).list();
		Integer [] vector = new Integer[elements.size()];
		int index = 0;
		for (Node i : elements) {
			Class type = (Class) dispatch(i);
			assert(type == Value.class);
			Value<Integer> val = (Value<Integer>) i.getProperty(Constants.TYPE);
			vector[index++] = val.value();
		}
		n.setProperty(Constants.TYPE, new Value<Integer[]>(vector));
		return Value.class;
	}

	public Class visitFloatVector(final GNode n) {
		List<Node> elements = n.<Node>getList(0).list();
		Float [] vector = new Float[elements.size()];
		int index = 0;
		for (Node f : n.<Node>getList(0)) {
			Class type = (Class) dispatch(f);
			assert(type == Value.class);
			Value<Float> val = (Value<Float>) f.getProperty(Constants.TYPE);
			vector[index++] = val.value();
		}
		n.setProperty(Constants.TYPE, new Value<Float[]>(vector));
		return Value.class;
	}

}
