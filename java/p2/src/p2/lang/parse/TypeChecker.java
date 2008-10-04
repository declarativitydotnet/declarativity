package p2.lang.parse;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import p2.core.Periodic;
import p2.lang.plan.Aggregate;
import p2.lang.plan.Alias;
import p2.lang.plan.ArrayIndex;
import p2.lang.plan.Assignment;
import p2.lang.plan.Cast;
import p2.lang.plan.DontCare;
import p2.lang.plan.Expression;
import p2.lang.plan.Fact;
import p2.lang.plan.GenericAggregate;
import p2.lang.plan.IfThenElse;
import p2.lang.plan.MethodCall;
import p2.lang.plan.Program;
import p2.lang.plan.UnknownReference;
import p2.lang.plan.NewClass;
import p2.lang.plan.Null;
import p2.lang.plan.Predicate;
import p2.lang.plan.Range;
import p2.lang.plan.Reference;
import p2.lang.plan.StaticReference;
import p2.lang.plan.Rule;
import p2.lang.plan.Selection;
import p2.lang.plan.StaticMethodCall;
import p2.lang.plan.Term;
import p2.lang.plan.Value;
import p2.lang.plan.Variable;
import p2.lang.plan.ObjectReference;
import p2.lang.plan.Watch;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Aggregation;
import p2.types.table.BasicTable;
import p2.types.table.EventTable;
import p2.types.table.Key;
import p2.types.table.Table;
import p2.types.table.TableName;
import xtc.Constants;
import xtc.tree.GNode;
import xtc.tree.Node;
import xtc.tree.Visitor;
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
	
	public static Class type(String name) {
		return NAME_TO_BASETYPE.containsKey(name) ? NAME_TO_BASETYPE.get(name) : null;
	}
	
	private Long uniqueID;

	/** The runtime. */
	protected Runtime runtime;
	
	protected Program program;

	/** The symbol table. */
	protected SymbolTable table;
	
	private Set<String> ruleNames;

	/**
	 * Create a new Overlog analyzer.
	 *
	 * @param runtime The runtime.
	 */
	public TypeChecker(Runtime runtime, Program program) {
		this.runtime = runtime;
		this.program = program;
	}
	
	public SymbolTable table() {
		return this.table;
	}
	
	public void prepare() {
		this.table = new SymbolTable(); 
		this.ruleNames = new HashSet<String>();
		this.uniqueID = 0L;
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
		if (x == y) {
			return x;
		}
		else if (y == null) {
			return x;
		}
		else if (x == null) {
			return y;
		}
		else if (x.isAssignableFrom(y)) {
			return x;
		} else if (y.isAssignableFrom(x)) {
			return y;
		} else {
			return lub(x.getSuperclass(), y);
		}
	}
	
	private boolean subtype(Class superType, Class subType) {
		if (subType == null) return true;
		
		if (type(superType.getSimpleName()) != null) superType = type(superType.getSimpleName()); 
		if (type(subType.getSimpleName()) != null) subType = type(subType.getSimpleName()); 
		
		for (Class inter : subType.getInterfaces()) {
			if (superType == inter || superType.isAssignableFrom(subType)) {
				return true;
			}
		}
		
		return superType == subType || superType.isAssignableFrom(subType);
	}

	// =========================================================================

	private p2.lang.plan.Boolean ensureBooleanValue(Expression expr) {
		if (expr.type() == null || expr.type() == Void.class) {
			runtime.error("Can't ensure boolean value from void class");
			return null; // Can't do it for void expression type
		}
		else if (Number.class.isAssignableFrom(expr.type())) {
			/* expr != 0 */
			return new p2.lang.plan.Boolean(p2.lang.plan.Boolean.NEQUAL,
					                    expr, new Value<Number>(0));
		}
		else if (expr.type() != boolean.class && !Boolean.class.isAssignableFrom(expr.type())) {
			/* expr != null*/
			return new p2.lang.plan.Boolean(p2.lang.plan.Boolean.NEQUAL,
									    expr, new Null());
		}
		else if (!p2.lang.plan.Boolean.class.isAssignableFrom(expr.getClass())) {
			return new p2.lang.plan.Boolean(p2.lang.plan.Boolean.EQUAL, expr, 
									    new Value<java.lang.Boolean>(java.lang.Boolean.TRUE));
		}
		return (p2.lang.plan.Boolean) expr;
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
	
	public Class visitImport(final GNode n) {
		Class type = (Class) dispatch((Node)n.getNode(0));
		if (type == Error.class) return Error.class;
		else if (type != Class.class) {
			runtime.error("Expected class type at import statement!", n);
			return Error.class;
		}
		type = (Class) n.getNode(0).getProperty(Constants.TYPE);
		
		type = (Class) n.getNode(0).getProperty(Constants.TYPE);
		NAME_TO_BASETYPE.put(type.getSimpleName(), type);
		n.setProperty(Constants.TYPE, type);
		return Class.class;
	}
	
	public Class visitFact(final GNode n) {
		Class type = (Class) dispatch((Node)n.getNode(0));
		assert(type == TableName.class);
		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
			if (Table.table(name) == null) {
				name.scope = Table.GLOBALSCOPE;
			}
		}
		if (Table.table(name) == null) {
			runtime.error("No table defined for fact " + name, n);
			return Error.class;
		}
		
		List<Expression> args = new ArrayList<Expression>();
		if (n.size() != 0) {
			for (Node arg : n.<Node> getList(1)) {
				Class t = (Class) dispatch(arg);
				assert(subtype(Expression.class, t));
				Expression expr = (Expression) arg.getProperty(Constants.TYPE);
				if (expr.variables().size() > 0) {
					runtime.error("Facts are not allowed to contain variables!", n);
					return Error.class;
				}
				args.add(expr);
			}
		}
		
		Fact fact = new Fact(n.getLocation(), name, args);
		n.setProperty(Constants.TYPE, fact);
		return Fact.class;
	}
	
	public Class visitWatch(final GNode n) {
		Class type = (Class) dispatch((Node)n.getNode(0));
		assert(type == TableName.class);
		
		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
			if (Table.table(name) == null) {
				name.scope = Table.GLOBALSCOPE;
			}
		}
		
		if (Table.table(name) == null) {
			runtime.error("Watch statement refers to unknown table/event name " + name, n);
			return Error.class;
		}
		String modifier = n.size() > 1 ? n.getString(1) : null;
		
		List<Watch> watches = new ArrayList<Watch>();
		if (modifier == null) {
			watches.add(new Watch(n.getLocation(), name, p2.types.operator.Watch.Modifier.NONE));
		}
		else {
			for (char mod : modifier.toCharArray()) {
				watches.add(new Watch(n.getLocation(), name, p2.types.operator.Watch.modifiers.get(mod)));
			}
			
		}
		n.setProperty(Constants.TYPE, watches);
		return Watch.class;
	}
	
	public Class visitTable(final GNode n) {
		Class type;

		/************** Table Name ******************/
		type = (Class) dispatch(n.getNode(0));
		assert(type == TableName.class);
		
		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
		}
		else {
			runtime.error("Dot specificed names not allowd in definition statement! " + n.getNode(0));
			return Error.class;
		}

		/************** Table Primary Key ******************/
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return type;
		assert (type == Key.class);
		Key key  = (Key) n.getNode(1).getProperty(Constants.TYPE);

		
		/************** Table Schema ******************/
		type = (Class) dispatch(n.getNode(2));
		if (type == Error.class) return type;
		assert(type == TypeList.class);
		TypeList schema  = (TypeList) n.getNode(2).getProperty(Constants.TYPE);

		Table create = new BasicTable(name, key, schema);
		this.program.definition(create);
		n.setProperty(Constants.TYPE, create);
		return Table.class;
	}

	
	public Class visitEvent(final GNode n) {
		Class type;

		/************** Event Name ******************/
		type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		assert(type == TableName.class);
		
		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
		} else {
			runtime.error("Dot specificed names not allowed in definition statement! " + n.getNode(0));
			return Error.class;
		}
		
		/************** Table Schema ******************/
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return type;
		assert(type == TypeList.class);
		TypeList schema  = (TypeList) n.getNode(1).getProperty(Constants.TYPE);

		EventTable event = new EventTable(name, schema);
		n.setProperty(Constants.TYPE, event);
		return Table.class;
	}
	
	public Class visitKeys(final GNode n) {
		List<Node> keyList = n.size() == 0 ? new ArrayList<Node>() : n.<Node>getList(0).list();
		Integer[] keys = new Integer[keyList.size()];
		int index = 0;
		if (keyList.size() > 0) {
			for (Node k : keyList) {
				Class type = (Class) dispatch(k);
				if (type == Error.class) return Error.class;
				assert(type == Value.class);
				
				Value key  = (Value) k.getProperty(Constants.TYPE);
				if (key.type() == null || !Integer.class.isAssignableFrom(key.type())) {
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
		for (Node attr : n.<Node>getList(0)) {
			Class type = (Class) dispatch(attr);
			if (type == Error.class) return Error.class;
			assert(type == Class.class);
			type = (Class) attr.getProperty(Constants.TYPE);
			types.add(type);
		}
		n.setProperty(Constants.TYPE, new TypeList(types));
		return TypeList.class;
	}
	
	public Class visitRule(final GNode n) {
		String name = (n.getString(1) == null) ? 
				      "Rule" + this.uniqueID++ : n.getString(1);
		
		if (ruleNames.contains(name)) {
			runtime.error("Multiple rule names defined as " 
					+ name + ". Rule name must be unique!");
			java.lang.System.exit(0);
			return Error.class;
		}
		ruleNames.add(name);
		
		Boolean isPublic = n.getString(0) != null;
		Boolean isDelete = n.getString(2) != null;
		
		
		Predicate head = null;
		List<Term> body = null;
		
		/* Rules define a scope for the variables defined. 
		 * The body has the highest scope followed by the
		 * head. */
		table.enter("Body:" + name);
		try {
			/* Evaluate the body first. */
			Class type = (Class) dispatch(n.getNode(4));
			
			if (type == Error.class) {
				return Error.class;
			}
			assert(type == List.class);
			body = (List<Term>) n.getNode(4).getProperty(Constants.TYPE);
			
			Term event = null;
			for (Term t : body) {
				if (t instanceof Predicate) {
					Predicate p = (Predicate) t;
					for (Expression arg : p) {
						if (arg instanceof Aggregate) {
							runtime.error("Body predicate can't contain aggregates!",n);
							return Error.class;
						}
					}
					
					Table table = Table.table(p.name());
					if (table.type() == Table.Type.EVENT ||
					    p.event() != Table.Event.NONE) {
						if (event != null) {
							runtime.error("Multiple event predicates in rule body!", n);
							return Error.class;
						}
						event = p;
					}
				}
				/*
				else if (t instanceof Function) {
					Function f = (Function) t;
					event = f.predicate();
				}
				*/
			}
			
			table.enter("Head:" + name);
			try {
				/* Evaluate the head. */
				type = (Class) dispatch(n.getNode(3));
				if (type == Error.class) {
					return Error.class;
				}
				assert (type == Predicate.class);
				head = (Predicate) n.getNode(3).getProperty(Constants.TYPE);
			} finally {
				table.exit();
			}
		} finally {
			table.exit();
		}
		
		Rule rule = new Rule(n.getLocation(), name, isPublic, isDelete, head, body);
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
		else if (head.event() != Table.Event.NONE) {
			runtime.error("Can't apply event modifier to rule head!", n);
			return Error.class;
		}

		/* All variables mentioned in the head must be in the body. 
		 * The types must also match. */
		int position = 0;
		for (Expression argument : head) {
			if (argument instanceof Variable) {
				Variable var = (Variable) argument;
				var.position(position);
				if (var instanceof DontCare) {
					runtime.error("Head predicate in a non-deletion rule " +
							      "can't contain don't care variable!", n);
					return Error.class;
				}
				else if (var instanceof Aggregate) {
					Aggregate agg = (Aggregate) var;
					Table table = Table.table(head.name());
					if (table instanceof Aggregation) {
						Aggregation aggregation = (Aggregation) table;
						if (!agg.functionName().equals(aggregation.variable().functionName()) ||
								!agg.name().equals(aggregation.variable().name())) {
							runtime.error("Table aggregates must be unique!", n);
							return Error.class;
						}
					}
					
					try {
						/* Drop the previous table. */
						Table.drop(table.name());
					} catch (UpdateException e) {
						runtime.error(e.toString());
						return Error.class;
					}
					Table aggregate = new Aggregation(head, table.type());
					this.program.definition(aggregate);
					
					Class[] types = table.types();
					if (types[var.position()] != var.type()) {
						runtime.error("Aggregate type "+ aggregate.type() + 
								      " does not match head type " + types[var.position()] + "!", n);
						return Error.class;
					}
				}
				else {
					Class headType = (Class) table.current().lookupLocally(var.name());
					Class bodyType = (Class) table.current().getParent().lookupLocally(var.name());
					if (bodyType == null) {
						for (Iterator<String> name = table.current().getParent().symbols(); name.hasNext(); )
							System.err.println(name.next());
						runtime.error("Head variable " + var
								+ " not defined in rule body.", n);
						return Error.class;
					} else if (!headType.isAssignableFrom(bodyType)) {
						runtime.error("Type mismatch: Head variable " + var
								+ " type is " + headType + ", but defined as type "
								+ bodyType + " in rule body.");
						return Error.class;
					}
				}
			}
			position++;
		}

		n.setProperty(Constants.TYPE, head);
		return Predicate.class;
	}

	public Class visitRuleBody(final GNode n) {
		List<Term> terms = new ArrayList<Term>();
		List<GNode> other = new ArrayList<GNode>();
		
		for (GNode node : n.<GNode>getList(0)) {
			if (node.getName().equals("TableFunction")) {
				Class type = (Class) dispatch(node);
				if (type == Error.class) {
					return type;
				}
				p2.lang.plan.Function f = 
					(p2.lang.plan.Function) node.getProperty(Constants.TYPE);
				terms.add(0, f);
			}
			else if (node.getName().equals("Predicate")) {
				Class type = (Class) dispatch(node);
				if (type == Error.class) {
					return type;
				}
				Predicate p = (Predicate) node.getProperty(Constants.TYPE);
				if (p.event() != Table.Event.NONE && p.notin()) {
					runtime.error("Can't apply notin to event predicate!",n);
					return Error.class;
				}
				terms.add(p);
			}
			else {
				other.add(node);
			}
		}
		
		for (GNode node : other) {
			Class type = (Class) dispatch(node);
			if (type == Error.class) {
				return type;
			}
			assert(Term.class.isAssignableFrom(type));
			terms.add((Term)node.getProperty(Constants.TYPE));
		}
		
		n.setProperty(Constants.TYPE, terms);
		return List.class;
	}
	
	public Class visitTableFunction(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		assert(type == TableName.class);
		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = Table.GLOBALSCOPE;
		}
		
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return Error.class;
		assert(type == Predicate.class);
		Predicate predicate = (Predicate) n.getNode(1).getProperty(Constants.TYPE);
		
		Table table = Table.table(name);
		if (table == null || table.type() != Table.Type.FUNCTION) {
			runtime.error("Unknown table function " + name, n);
			return Error.class;
		}
		else {
			/* Make sure predicate schema matches table function schema. */
			Class[] types = table.types();
			int index = 0;
			for (Expression arg : predicate) {
				if (types[index] != arg.type()) {
					runtime.error("Predicate argument position " + index + " does not match table function type!",n);
					return Error.class;
				}
				index++;
			}
			if (types.length != index) {
				runtime.error("Predicate schema types must match table function types!",n);
				return Error.class;
			}
		}
		
		n.setProperty(Constants.TYPE, new p2.lang.plan.Function(table, predicate));
		return p2.lang.plan.Function.class;
	}

	public Class visitPredicate(final GNode n) {
		boolean notin = n.getString(0) != null;
		String event = n.getString(2);
		
		Class type = (Class) dispatch(n.getNode(1));
		assert(type == TableName.class);
		TableName name = (TableName) n.getNode(1).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
			if (Table.table(name) == null) {
				name.scope = Table.GLOBALSCOPE;
			}
		}
		
		/* Lookup the schema for the given tuple name. */
		Table ptable = Table.table(name);
		if (ptable == null) {
			runtime.error("No catalog definition for predicate " + name, n);
			return Error.class;
		}
		
		Table.Event tableEvent = Table.Event.NONE;
		if (!(ptable instanceof EventTable) && event != null) {
			if ("insert".equals(event)) {
				tableEvent = Table.Event.INSERT;
			}
			else if ("delete".equals(event)) {
				tableEvent = Table.Event.DELETE;
			}
			else {
				runtime.error("Unknown event modifier " + event + " on predicate " + name, n);
			}
		}
		
		TypeList schema = new TypeList(ptable.types());
		type = (Class) dispatch(n.getNode(3));
		if (type == Error.class) 
			return Error.class;
		List<Expression> parameters = (List<Expression>) n.getNode(3).getProperty(Constants.TYPE);
		List<Expression> arguments = new ArrayList<Expression>();
		

		if (!"runtime".equals(this.program.name()) && "periodic".equals(name.name)) {
			if (parameters.size() < 1 || parameters.size() > Periodic.SCHEMA.length) {
				runtime.error("Too many arguments to periodic predicate", n);
				return Error.class;
			}
			else if (!(parameters.get(0) instanceof Variable)) {
				runtime.error("First parameter to periodic predicate must be a Variable", n);
				return Error.class;
			}
			
			int index = 1;
			for ( ; index < parameters.size(); index++) {
				if (!(parameters.get(index) instanceof Value) ||
						!Number.class.isAssignableFrom(parameters.get(index).type())) {
					runtime.error("Parameter " + index + 
							      " to periodic predicate must be a Numeric value!", n);
					return Error.class;
				}
			}
			
			if (index == Periodic.Field.PERIOD.ordinal()) { // Period
				parameters.add(new Value<Long>(1L));
				index++;
			}
			else {
				Long period = ((Value<Long>)parameters.get(Periodic.Field.PERIOD.ordinal())).value();
				if (period < 1) {
					runtime.warning("Periodic period must be > 1. Setting to 1",n);
					period = 1L;
				}
				parameters.set(Periodic.Field.PERIOD.ordinal(), 
						new Value<Long>(period));
				
			}
			
			if (index == Periodic.Field.TTL.ordinal()) { // TTL or Max execution count
				parameters.add(new Value<Long>(Long.MAX_VALUE));
				index++;
			}
			
			if (index == Periodic.Field.TIME.ordinal()) { // Start time
				parameters.add(new Value<Long>(0L));
				index++;
			}
			else {
				Long offset = ((Value<Long>)parameters.get(Periodic.Field.TIME.ordinal())).value();
				if (offset < 0) {
					runtime.warning("Periodic offset must be > 0. Setting to 0",n);
					parameters.set(Periodic.Field.PERIOD.ordinal(),  new Value<Long>(0L));
				}
			}
			
			if (index == Periodic.Field.COUNT.ordinal()) { // Execution Count
				parameters.add(new Value<Long>(new Long(0)));
				index++;
			}
			

			if (index == Periodic.Field.PROGRAM.ordinal()) { // Program name
				parameters.add(new Value<String>(this.program.name()));
				index++;
			}
			tableEvent = Table.Event.INSERT;
		}
		else if (schema.size() < parameters.size()) {
			runtime.error("Program " + program.name() + ": Schema size mismatch on predicate " +  name ,n);
			return Error.class;
		}
		
		
		/* Type check each tuple argument according to the schema. */
		for (int index = 0; index < schema.size(); index++) {
			Expression param = parameters.size() <= index ? 
					new DontCare(schema.get(index)) : parameters.get(index);
			Class paramType = param.type();
			
			if (param instanceof Cast) {
				param = ((Cast)param).expression();
			}
			
			if (param instanceof Alias) {
				Alias alias = (Alias) param;
				if (alias.position() < index) {
					runtime.error("Alias fields must be in numeric order!");
					return Error.class;
				}
				/* Fill in missing variables with tmp variables. */
				while (index < alias.position()) {
					Variable dontcare = new DontCare(schema.get(index));
					dontcare.position(index++);
					arguments.add(dontcare);
				}
			}

			if (param instanceof Variable) {
				/* Only look in the current scope. */
				Variable var = (Variable) param;
				var.type(paramType);
				if (var.type() == null) {
					/* Fill in type using the schema. */
					var.type(schema.get(index));
				}

				/* Map variable to its type. */
				table.current().define(var.name(), var.type());
				paramType = var.type();
			}
			else if (param.variables().size() > 0) {
				for (Variable var : param.variables()) {
					if (!table.current().isDefined(var.name())) {
						runtime.error("Body predicate contains an argument expression based " +
									"that depends on variable " + var.name() + ", which is not" +
									"defined by some left hand side predicate!", n);
						return Error.class;
					}
				}
			}

			/* Ensure the type matches the schema definition. */
			if (!subtype(schema.get(index), paramType)) {
				runtime.error("Predicate " + name + " argument " + param.getClass() + " " + param
								+ " has type " + param.type() + " does not match type " 
								+ schema.get(index) + " in schema.", n);
				return Error.class;
			}

			param.position(index);
			arguments.add(param);
		}
		
		if (schema.size() != arguments.size()) {
			runtime.error("Schema size mismatch on predicate " +  name ,n);
			return Error.class;
		}
		
		Predicate pred = new Predicate(notin, name, tableEvent, arguments);
		
		pred.location(n.getLocation());
		n.setProperty(Constants.TYPE, pred);
		return Predicate.class;
	}

	public Class visitTableName(final GNode n) {
		String scope = n.size() > 1 ? n.getString(0) : null;
		String name  = n.size() > 1 ? n.getString(1) : n.getString(0);
		
		n.setProperty(Constants.TYPE, new TableName(scope, name));
		return TableName.class;
	}

	public Class visitAssignment(final GNode n) {
		/* Variable. */
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		if (!(type == Variable.class)) {
			runtime.error("Cannot assign to type " + type + 
					" must be of type Variable or Location variable!");
			return Error.class;
		}
		Variable var = (Variable) n.getNode(0).getProperty(Constants.TYPE);

		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
		Expression expr = (Expression) n.getNode(1).getProperty(Constants.TYPE);

		if (expr.type() != null && var.type() == null) {
			var.type(expr.type());
			table.current().define(var.name(), expr.type());
		}
		else if (!subtype(var.type(), expr.type())) {
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
		else if (!Expression.class.isAssignableFrom(type)) {
			runtime.error("Expected expression type but got type " + 
					      type + " instead.",n);
			return Error.class;
		}
		Expression expr = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		expr.location(n.getLocation()); // Set the location of this expression.
		n.setProperty(Constants.TYPE, n.getNode(0).getProperty(Constants.TYPE));
		return type;
	}
	
	public Class visitIfElseExpression(final GNode n) {
		Class iftype   = (Class) dispatch(n.getNode(0));
		if (iftype == Error.class) return Error.class;
		Class thentype = (Class) dispatch(n.getNode(1));
		if (thentype == Error.class) return Error.class;
		Class elsetype = (Class) dispatch(n.getNode(2));
		if (elsetype == Error.class) return Error.class;
		
		Expression ifexpr   = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression thenexpr = (Expression) n.getNode(1).getProperty(Constants.TYPE);
		Expression elseexpr = (Expression) n.getNode(2).getProperty(Constants.TYPE);
		
		if (ifexpr.type() == null) {
			runtime.error("Undefined expression " + ifexpr, n);
			return Error.class;
		}
		else if (thentype != Null.class && thenexpr.type() == null) {
			runtime.error("Undefined expression " + thenexpr, n);
			return Error.class;
		}
		else if (elsetype != Null.class && elseexpr.type() == null) {
			runtime.error("Undefined expression " + elseexpr, n);
			return Error.class;
		}
		else if (ensureBooleanValue(ifexpr) == null) {
			runtime.error("Cannot evaluate type " + ifexpr.type()
					+ " in a logical or expression", n);
			return Error.class;
		}
		
		n.setProperty(Constants.TYPE, 
				      new IfThenElse(lub(thenexpr.type(), elseexpr.type()), 
				    		         ensureBooleanValue(ifexpr), 
				    		         thenexpr, elseexpr));
		return IfThenElse.class;
	}

	public Class visitLogicalOrExpression(final GNode n) {
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(1));
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(1).getProperty(Constants.TYPE);

		if (lhs.type() == null) {
			runtime.error("Undefined expression " + lhs, n);
			return Error.class;
		}
		else if (rhs.type() == null) {
			runtime.error("Undefined expression " + rhs, n);
			return Error.class;
		}
		else if (ensureBooleanValue(lhs) == null) {
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
		Class rtype = (Class) dispatch(n.getNode(1));
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(1).getProperty(Constants.TYPE);

		if (lhs.type() == null) {
			runtime.error("Undefined expression " + lhs, n);
			return Error.class;
		}
		else if (rhs.type() == null) {
			runtime.error("Undefined expression " + rhs, n);
			return Error.class;
		}
		else if (ensureBooleanValue(lhs) == null) {
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

		if (ltype != Null.class && lhs.type() == null) {
			runtime.error("Undefined expression " + lhs, n);
			return Error.class;
		}
		else if (rtype != Null.class && rhs.type() == null) {
			runtime.error("Undefined expression " + rhs, n);
			return Error.class;
		}
		else if (Void.class == lhs.type()) {
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
	
	public Class visitInequalityExpression(final GNode n) {
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

		if (ltype != Null.class && lhs.type() == null) {
			runtime.error("Undefined expression " + lhs, n);
			return Error.class;
		}
		else if (rtype != Null.class && rhs.type() == null) {
			runtime.error("Undefined expression " + rhs, n);
			return Error.class;
		}
		else if (Void.class == lhs.type()) {
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
	
	public Class visitCastExpression(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return Error.class;
		
		Class cast = (Class) n.getNode(0).getProperty(Constants.TYPE);
		Expression expr = (Expression) n.getNode(1).getProperty(Constants.TYPE);
		
		if (expr.type() != null && !(subtype(expr.type(), cast) || subtype(cast, expr.type()))) {
			runtime.error("CAST ERROR: Expression type " + expr.type() + " is not a supertype of " + cast + ".", n);
			return Error.class;
		}
		
		n.setProperty(Constants.TYPE, new Cast(cast, expr));
		return Cast.class;
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
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
		type = (Class) dispatch(n.getNode(2));
		if (type == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(type));
		
		Expression begin = (Expression) n.getNode(1).getProperty(Constants.TYPE);
		Expression end   = (Expression) n.getNode(2).getProperty(Constants.TYPE);
		
		if (begin.type() == null) {
			runtime.error("Undefined expression " + begin, n);
			return Error.class;
		}
		else if (end.type() == null) {
			runtime.error("Undefined expression " + end, n);
			return Error.class;
		}
		else if (begin.type() != end.type()) {
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
		if (ltype == Error.class || rtype == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);
		
		if (lhs.type() == null) {
			runtime.error("Undefined expression " + lhs, n);
			return Error.class;
		}
		else if (rhs.type() == null) {
			runtime.error("Undefined expression " + rhs, n);
			return Error.class;
		}
		else if (lhs.type() == Integer.class && rhs.type() == Integer.class) {
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
		if (ltype == Error.class || rtype == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		String oper = n.getString(1);
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);

		if (lhs.type() == null) {
			runtime.error("Undefined expression " + lhs, n);
			return Error.class;
		}
		else if (rhs.type() == null) {
			runtime.error("Undefined expression " + rhs, n);
			return Error.class;
		}
		else if (Number.class.isAssignableFrom(lhs.type()) && 
			     Number.class.isAssignableFrom(rhs.type())) {
			n.setProperty(Constants.TYPE, new p2.lang.plan.Math(oper, lhs, rhs));
			return p2.lang.plan.Math.class;
		}
		else if (String.class.isAssignableFrom(lhs.type()) && 
			     String.class.isAssignableFrom(rhs.type())) {
			n.setProperty(Constants.TYPE, new p2.lang.plan.Math(oper, lhs, rhs));
			return p2.lang.plan.Math.class;
		}
		else if (Set.class.isAssignableFrom(lhs.type()) && 
			     Set.class.isAssignableFrom(rhs.type())) {
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
		if (ltype == Error.class || rtype == Error.class) return Error.class;
		assert(Expression.class.isAssignableFrom(ltype) && 
			   Expression.class.isAssignableFrom(rtype));
		
		String oper = n.getString(1);
		Expression lhs = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		Expression rhs = (Expression) n.getNode(2).getProperty(Constants.TYPE);

		if (lhs.type() == null) {
			runtime.error("Undefined expression " + lhs, n);
			return Error.class;
		}
		else if (rhs.type() == null) {
			runtime.error("Undefined expression " + rhs, n);
			return Error.class;
		}
		else if (Number.class.isAssignableFrom(lhs.type()) && 
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
		Expression context = (Expression) n.getNode(0).getProperty(Constants.TYPE);
		
		Class argumentType = (Class) dispatch(n.getNode(1)); 
		assert(argumentType == List.class);
		List<Expression> arguments = 
			(List<Expression>) n.getNode(1).getProperty(Constants.TYPE);
		List<Class> parameterTypes = new ArrayList<Class>();
		for (Expression arg : arguments) {
			parameterTypes.add(arg.type());
		}
		
		try {
			Class [] types = parameterTypes.toArray(new Class[parameterTypes.size()]);
			if (type == NewClass.class) {
				NewClass newclass = (NewClass) context;
				Constructor constructor = newclass.type().getConstructor(types);
				newclass.constructor(constructor);
				newclass.arguments(arguments);
				n.setProperty(Constants.TYPE, newclass);
				return NewClass.class;
			}
			else if (type == Reference.class) {
				Reference reference = (Reference) context;
				if (reference.object() == null && reference.type() == null) {
					runtime.error("Undefined reference " + reference.toString(), n);
					return Error.class;
				}
				else if (reference.object() != null) {
					Expression object = reference.object();
					if (object.type() == null) {
						runtime.error("Undefined expression object: " + object, n);
						return Error.class;
					}
					
				    Method method = object.type().getMethod(reference.toString(), types);
				    n.setProperty(Constants.TYPE, new MethodCall(object, method, arguments));
				    return MethodCall.class;
				}
				else {
					Method method = reference.type().getMethod(reference.toString(), types);
			// XXX there should be a check here, but this check fails on static method invocations.
			//		if (method.getModifiers() != Modifier.STATIC) {
			//			runtime.error("Expected method " + reference.toString() + " to be static!", n);
			//			return Error.class;
			//		}
					n.setProperty(Constants.TYPE, new StaticMethodCall(reference.type(), method, arguments));
					return StaticMethodCall.class;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			runtime.error("Method error: on " +  context.toString() + ": " + e.toString(), n);
			return Error.class;
		}
		
		runtime.error("Unkown method reference " + context.toString(), n);
		return Error.class;
	}
	
	public Class visitNewClass(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		type = (Class) n.getNode(0).getProperty(Constants.TYPE);
		
		n.setProperty(Constants.TYPE, new NewClass(type));
		return NewClass.class;
	}
	
	public Class visitReference(final GNode n)  {
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		
		if (type == Variable.class) {
			Variable var = (Variable) n.getNode(0).getProperty(Constants.TYPE);
			if (type(var.name()) != null) {
				runtime.warning("Assuming " + var.name() + 
						        " is not a variable but rather refers the class type of " + 
						        type(var.name()), n);
				n.getNode(0).setProperty(Constants.TYPE, type(var.name()));
				type = Class.class;
			}
		}
		
		if (type == Reference.class) {
			Reference ref = (Reference) n.getNode(0).getProperty(Constants.TYPE);
			if (ref.object() != null || ref.type() != null) {
				runtime.error("Undefined reference " + ref.toString(), n);
				return Error.class;
			}
			String name = ref.toString() + "." + n.getString(1);
			
			if (type(name) != null) {
				type = type(name);
				n.setProperty(Constants.TYPE, type);
				return Class.class;
			}
			
			try {
				type = Class.forName(name);
				n.setProperty(Constants.TYPE, type);
				return Class.class;
			} catch (ClassNotFoundException e) {
				n.setProperty(Constants.TYPE, new UnknownReference(null, null, name));
				return Reference.class;
			}
		}
		else if (Expression.class.isAssignableFrom(type)) {
			Expression expr = (Expression) n.getNode(0).getProperty(Constants.TYPE);
			String name = n.getString(1);
			type = expr.type();
			try {
				Field field = type.getField(name);
				n.setProperty(Constants.TYPE, new ObjectReference(expr, field));
				return ObjectReference.class;
			} catch (Exception e) { 
				n.setProperty(Constants.TYPE, new UnknownReference(expr, null, name));
				return Reference.class;
			}
		}
		else if (type == Class.class) {
			/* Static reference or method. */
			type = (Class) n.getNode(0).getProperty(Constants.TYPE);
			String name = n.getString(1);
			
			/* Check if it's a subclass. */
			for (Class sub : type.getClasses()) {
				if (sub.getSimpleName().equals(name)) {
					n.setProperty(Constants.TYPE, sub);
					return Class.class;
				}
			}
			
			try {
				/* Check if it's a static field. */
				Field field = type.getField(name);
				if (!field.isEnumConstant() &&
					field.getModifiers() != Modifier.STATIC) {
					runtime.error("Field must be static or an enumeration!", n);
					return Error.class;
				}
				n.setProperty(Constants.TYPE, new StaticReference(type, field));
				return StaticReference.class;
			} catch (Exception e) {
				/* It must be a method at this point. Punt to higher ground. */
				n.setProperty(Constants.TYPE, new UnknownReference(null, type, name));
				return Reference.class;
			}
		}
		else {
			System.err.println("Unhandled reference type!");
			System.exit(0);
		}
		return Error.class;
	}
	
	public Class visitReferenceName(final GNode n) {
		n.setProperty(Constants.TYPE, new UnknownReference(null, null, n.getString(0)));
		return Reference.class;
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
				if (t == Error.class) { 
					return Error.class;
				}
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
		if (type == Array.class) {
			dispatch(n.getNode(1));
			int[] dim = (int[]) n.getNode(1).getProperty(Constants.TYPE);
			Object instance = Array.newInstance(type, dim);
			type = instance.getClass();
		}
		else {
			type = (Class) n.getNode(0).getProperty(Constants.TYPE);
		}
		
		n.setProperty(Constants.TYPE, type);
		return Class.class;
	}
	
	public Class visitDimensions(final GNode n) {
		int[] dims = new int[n.size()];
		n.setProperty(Constants.TYPE, dims);
		return Array.class;
	}

	public Class visitClassType(final GNode n) {
		try {
			String name = n.getString(0);
			for (Object c : n.getList(1)) {
				name  += "." + c.toString();
			}
			Class type = Error.class;
			if (type(name) != null) {
				type = type(name);
			}
			else {
				type = Class.forName(name);
			}
			n.setProperty(Constants.TYPE, type);
			return Class.class;
		} catch (ClassNotFoundException e) {
			runtime.error(e.toString(), n);
		}
		return Error.class;
	}

	public Class visitPrimitiveType(final GNode n) {
		Class type = type(n.getString(0));
		n.setProperty(Constants.TYPE, type);
		return Class.class;
	}

	public Class visitVariable(final GNode n) {
		Class type =  (Class)  table.current().lookup(n.getString(0));
		if (DontCare.DONTCARE.equals(n.getString(0))) {
			/* Generate a fake variable for all don't cares. */
			n.setProperty(Constants.TYPE, new DontCare(null));
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
		n.setProperty(Constants.TYPE, new Variable(var.name(), var.type(), true));
		return Variable.class;
	}

	public Class visitAggregate(final GNode n) {
		String function = n.getString(0);
		
		if ("generic".equals(function)) {
			Class type = (Class) dispatch(n.getNode(1));
			
			if (type != MethodCall.class) {
				runtime.error("Generic function must be a method call!!", n);
				return Error.class;
			}
			MethodCall method = (MethodCall) n.getNode(1).getProperty(Constants.TYPE);
			n.setProperty(Constants.TYPE, new GenericAggregate(method));
		}
		else {
			if (n.getNode(1) == null) {
				n.setProperty(Constants.TYPE, 
						new Aggregate((String)null, function, p2.types.function.Aggregate.type(function, null)));
			}
			else {
				Class type = (Class) dispatch(n.getNode(1));
				if (type == Variable.class) {
					Variable var = (Variable) n.getNode(1).getProperty(Constants.TYPE);
					n.setProperty(Constants.TYPE, 
							new Aggregate(var.name(), function, p2.types.function.Aggregate.type(function, var.type())));
				} else if (type == MethodCall.class) {
					MethodCall method = (MethodCall) n.getNode(1).getProperty(Constants.TYPE);
					n.setProperty(Constants.TYPE,  
							new Aggregate(method, function, p2.types.function.Aggregate.type(function, method.type())));
				}
			}
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
	
	public Class visitLongConstant(final GNode n) {
		n.setProperty(Constants.TYPE, new Value<Long>(Long.parseLong(n.getString(0))));
		return Value.class;
	}

	public Class visitStringConstant(final GNode n) {
		String value = n.getString(0);
		if (value.length() == 2) value = "";
		else value = value.substring(1, value.length()-1);
		n.setProperty(Constants.TYPE, new Value<String>(value));
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
		n.setProperty(Constants.TYPE, new Null());
		return Null.class;
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
