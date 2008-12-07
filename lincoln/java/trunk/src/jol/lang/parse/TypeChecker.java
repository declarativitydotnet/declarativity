package jol.lang.parse;

import java.io.File;
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

import jol.lang.plan.Aggregate;
import jol.lang.plan.AggregateVariable;
import jol.lang.plan.Alias;
import jol.lang.plan.ArrayIndex;
import jol.lang.plan.Assignment;
import jol.lang.plan.BottomK;
import jol.lang.plan.Cast;
import jol.lang.plan.DontCare;
import jol.lang.plan.Expression;
import jol.lang.plan.Fact;
import jol.lang.plan.GenericAggregate;
import jol.lang.plan.IfThenElse;
import jol.lang.plan.Limit;
import jol.lang.plan.Load;
import jol.lang.plan.MethodCall;
import jol.lang.plan.NewClass;
import jol.lang.plan.Null;
import jol.lang.plan.ObjectReference;
import jol.lang.plan.Predicate;
import jol.lang.plan.Program;
import jol.lang.plan.Range;
import jol.lang.plan.Reference;
import jol.lang.plan.Rule;
import jol.lang.plan.Selection;
import jol.lang.plan.StaticMethodCall;
import jol.lang.plan.StaticReference;
import jol.lang.plan.Term;
import jol.lang.plan.TopK;
import jol.lang.plan.UnknownReference;
import jol.lang.plan.Value;
import jol.lang.plan.VList;
import jol.lang.plan.Variable;
import jol.lang.plan.Watch;
import jol.lang.plan.Fact.FactTable;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.basic.ValueList;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.table.Aggregation;
import jol.types.table.BasicTable;
import jol.types.table.EventTable;
import jol.types.table.Flatten;
import jol.types.table.Key;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.TimerTable;
import xtc.Constants;
import xtc.tree.GNode;
import xtc.tree.Node;
import xtc.tree.Visitor;
import xtc.util.SymbolTable;

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

	/** The runtime context */
	protected jol.core.Runtime context;

	/** The runtime. */
	protected xtc.util.Runtime runtime;

	protected Program program;

	/** The symbol table. */
	protected SymbolTable table;

	private Set<String> ruleNames;

	/**
	 * Create a new Overlog analyzer.
	 *
	 * @param runtime The runtime.
	 */
	public TypeChecker(jol.core.Runtime context, xtc.util.Runtime runtime, Program program) {
		this.context = context;
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
	 * @param node The root node.
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
	private Class<?> lub(final Class<?> x, final Class<?> y) {
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

	private boolean subtype(Class<?> superType, Class<?> subType) {
		if (subType == null) return true;
		if (superType == null) {
			System.err.println("FATAL COMPILE ERROR: super type check null! subtype " + subType);
		}

		if (type(superType.getSimpleName()) != null) superType = type(superType.getSimpleName());
		if (type(subType.getSimpleName()) != null) subType = type(subType.getSimpleName());

		for (Class inter : subType.getInterfaces()) {
			if (superType == inter || superType.isAssignableFrom(subType)) {
				return true;
			}
		}

		return superType == subType || superType.isAssignableFrom(subType);
	}

	private boolean typeCoercion(Class[] formalTypes, Class[] argumentTypes) {
		if (argumentTypes.length != formalTypes.length) return false;

		for (int i = 0; i < argumentTypes.length; i++) {
			if (!subtype(formalTypes[i], argumentTypes[i]))  {
				return false;
			}
		}
		return true;
	}

	// =========================================================================

	private jol.lang.plan.Boolean<?> ensureBooleanValue(Expression expr) {
		if (expr.type() == null || expr.type() == Void.class) {
			runtime.error("Can't ensure boolean value from void class");
			return null; // Can't do it for void expression type
		}
		else if (subtype(Number.class, expr.type())) {
			/* expr != 0 */
			return new jol.lang.plan.Boolean(jol.lang.plan.Boolean.NEQUAL,
					                    expr, new Value<Number>(0));
		}
		else if (!subtype(Boolean.class, expr.type())) {
			/* expr != null*/
			return new jol.lang.plan.Boolean(jol.lang.plan.Boolean.NEQUAL,
									    expr, new Null());
		}
		else if (!jol.lang.plan.Boolean.class.isAssignableFrom(expr.getClass())) {
			return new jol.lang.plan.Boolean(jol.lang.plan.Boolean.EQUAL, expr,
									    new Value<java.lang.Boolean>(java.lang.Boolean.TRUE));
		}
		return (jol.lang.plan.Boolean) expr;
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
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		else if (type != Class.class) {
			runtime.error("Expected class type at import statement!", n);
			return Error.class;
		}
		type = (Class) n.getNode(0).getProperty(Constants.TYPE);
		NAME_TO_BASETYPE.put(type.getSimpleName(), type);
		n.setProperty(Constants.TYPE, type);
		return Class.class;
	}

	public Class visitLoad(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		assert(type == Value.class);
		Value<?> fileName = (Value<?>) n.getNode(0).getProperty(Constants.TYPE);
		File file = new File((String)fileName.value());

		type = (Class) dispatch(n.getNode(1));
		assert(type == TableName.class);
		TableName name = (TableName) n.getNode(1).getProperty(Constants.TYPE);

		String delim = null;
		if (n.size() > 2) {
			type = (Class) dispatch(n.getNode(2));
			Value<?> delimValue = (Value<?>) n.getNode(2).getProperty(Constants.TYPE);
		    delim = (String)delimValue.value();
		}

		if (name.scope == null) {
			name.scope = program.name();
			if (context.catalog().table(name) == null) {
				name.scope = Table.GLOBALSCOPE;
			}
		}
		if (context.catalog().table(name) == null) {
			runtime.error("No table definition found for load on " + name, n);
			return Error.class;
		}
		else if (!file.exists()) {
			runtime.warning("Loader: unknown file " + file.getAbsolutePath(), n);
			return null;
		}

		Load loader = new Load(n.getLocation(), file, context.catalog().table(name), delim);
		n.setProperty(Constants.TYPE, loader);
		return Load.class;
	}

	public Class visitFact(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		assert(type == TableName.class);
		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
			if (context.catalog().table(name) == null) {
				name.scope = Table.GLOBALSCOPE;
			}
		}

		Table table = context.catalog().table(name);
		if (table == null) {
			runtime.error("No table defined for fact " + name, n);
			return Error.class;
		}

		Class[] types = table.types();
		List<Expression> args = new ArrayList<Expression>();

		if (n.size() != 0) {
			int index = 0;
			for (Node arg : n.<Node> getList(1)) {
				Class t = (Class) dispatch(arg);
				Expression expr = (Expression) arg.getProperty(Constants.TYPE);
				if (expr.variables().size() > 0) {
					runtime.error("Facts are not allowed to contain variables!", n);
					return Error.class;
				}
				else if (index >= types.length) {
					break;
				}
				else if (!subtype(types[index], expr.type())) {
					runtime.error("Type mismatch with fact argument "
							      + index + " of type " + expr.type() +
							      ", expected type " + types[index], n);
					return Error.class;
				}
				args.add(expr);
				index++;
			}
		}

		if (types.length != args.size()) {
			runtime.error("Argument mismatch in facts for table " + name +
					      ". Table has " + types.length + ", but facts has size " +
					      args.size() + ".", n);
			return Error.class;
		}

		Fact fact = new Fact(n.getLocation(), name, args);
		n.setProperty(Constants.TYPE, fact);
		return Fact.class;
	}

	public Class visitWatch(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		assert(type == TableName.class);

		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
			if (context.catalog().table(name) == null) {
				name.scope = Table.GLOBALSCOPE;
			}
		}

		if (context.catalog().table(name) == null) {
			runtime.error("Watch statement refers to unknown table/event name " + name, n);
			return Error.class;
		}
		String modifier = n.size() > 1 ? n.getString(1) : null;

		List<Watch> watches = new ArrayList<Watch>();
		if (modifier == null) {
			watches.add(new Watch(n.getLocation(), name, jol.types.operator.Watch.Modifier.NONE));
		}
		else {
			for (char mod : modifier.toCharArray()) {
				watches.add(new Watch(n.getLocation(), name, jol.types.operator.Watch.modifiers.get(mod)));
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
			runtime.error("Dot specificed names not allowed in definition statement! " + n.getNode(0));
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
		Table create;
		/*
		if (name.name.startsWith("stasis")) {
			//create = new JavaHashtable(context, name, key, schema);
			try {
				// create = new LinearHashNTA(context, name, key, schema);
			} catch (UpdateException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				create = null;
			}
		} else {
			create = new BasicTable(context, name, key, schema);
		}
		*/
		create = new BasicTable(context, name, key, schema);
		context.catalog().register(create);
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
		context.catalog().register(event);
		n.setProperty(Constants.TYPE, event);
		return Table.class;
	}

	public Class visitTimer(final GNode n) {
		Class type;

		/************** Timer Name ******************/
		type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;

		TableName name = (TableName) n.getNode(0).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
		} else {
			runtime.error("Dot specificed names not allowed in definition statement! " + n.getNode(0));
			return Error.class;
		}

		String timerType = n.getString(1);

		type = (Class) dispatch(n.getNode(2));
		if (type == Error.class) return Error.class;
		Value<Integer> period = (Value) n.getNode(2).getProperty(Constants.TYPE);

		type = (Class) dispatch(n.getNode(3));
		if (type == Error.class) return Error.class;
		Value<Integer> ttl = (Value) n.getNode(3).getProperty(Constants.TYPE);

		type = (Class) dispatch(n.getNode(4));
		if (type == Error.class) return Error.class;
		Value<Integer> delay = (Value) n.getNode(4).getProperty(Constants.TYPE);

		try {
			TimerTable timer =
				new TimerTable(context, name, timerType,
					           period.value().longValue(),
					           ttl.value().longValue(),
					           delay.value().longValue());

			/* Create a fact that will insert the timer into the timer table, basically starting the timer. */
			context.catalog().table(FactTable.TABLENAME).force(
					new Tuple(program.name(), name, timer.tuples().iterator().next()));
			context.catalog().register(timer);
			n.setProperty(Constants.TYPE, timer);
		} catch (UpdateException e) {
			runtime.error("Unable to schedule initial " + name + " timer!", n);
			return Error.class;
		}
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

				Value<?> key  = (Value<?>) k.getProperty(Constants.TYPE);
				if (key.type() == null || !Integer.class.isAssignableFrom(key.type())) {
					runtime.error("Key must be of type Integer! type = " + key);
					return Error.class;
				}
				keys[index++] = (Integer)key.value();
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
			type = (Class) attr.getProperty(Constants.TYPE);
			types.add(type);
		}
		n.setProperty(Constants.TYPE, new TypeList(types));
		return TypeList.class;
	}

	public Class visitRule(final GNode n) {
		String name = (n.getString(2) == null) ?
				      "Rule" + this.uniqueID++ : n.getString(2);

		if (ruleNames.contains(name)) {
			runtime.error("Multiple rule names defined as "
					+ name + ". Rule name must be unique!");
			java.lang.System.exit(0);
			return Error.class;
		}
		ruleNames.add(name);

		Boolean isPublic = n.getString(0) != null;
		Boolean isAsync  = n.getString(1) != null;
		Boolean isDelete = n.getString(3) != null;

		Predicate head = null;
		List<Term> body = null;

		/* Rules define a scope for the variables defined.
		 * The body has the highest scope followed by the
		 * head. */
		table.enter("Body:" + name);
		try {
			/* Evaluate the body first. */
			Class type = (Class) dispatch(n.getNode(5));

			if (type == Error.class) {
				return Error.class;
			}
			assert(type == List.class);
			body = (List<Term>) n.getNode(5).getProperty(Constants.TYPE);

			Term event = null;
			for (Term t : body) {
				if (t instanceof Predicate) {
					Predicate p = (Predicate) t;
					for (Expression arg : p) {
						if (arg instanceof Aggregate) {
							runtime.error("Body predicate can't contain aggregates!", n);
							return Error.class;
						}
					}

					Table table = context.catalog().table(p.name());
					if (table.type() == Table.Type.EVENT ||
					    p.event() != Predicate.Event.NONE) {
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
				type = (Class) dispatch(n.getNode(4));
				if (type == Error.class) {
					return Error.class;
				}
				assert (type == Predicate.class);
				head = (Predicate) n.getNode(4).getProperty(Constants.TYPE);
			} finally {
				table.exit();
			}
		} finally {
			table.exit();
		}

		Rule rule = new Rule(n.getLocation(), name, isPublic, isAsync, isDelete, head, body);
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
		else if (head.event() != Predicate.Event.NONE) {
			runtime.error("Can't apply event modifier to rule head!", n);
			return Error.class;
		}

		/* All variables mentioned in the head must be in the body.
		 * The types must also match. */
		for (int position = 0; position < head.arguments().size(); position++) {
			Expression argument = head.argument(position);
			if (argument instanceof Aggregate) {
				Aggregate agg = (Aggregate) argument;
				Table table = context.catalog().table(head.name());
				if (!(table instanceof Aggregation)) {
					try {
						/* Drop the previous table. */
						context.catalog().drop(table.name());
					} catch (UpdateException e) {
						runtime.error(e.toString());
						return Error.class;
					}
					try {
						table = new Aggregation(context, head, table.type(), table.types());
					} catch (PlannerException e) {
						runtime.error(e.toString(), n);
						return Error.class;
					}
					context.catalog().register(table);
					this.program.definition(table);
				}
				else {
					// TODO: Verify that table aggregate is compatible!
				}

				Class[] types = table.types();
				if (!subtype(types[position], argument.type())) {
					runtime.error("Aggregate " + argument + ", position " + position +
							", type "+ agg.type() +
							      " does not match table type " +
							      types[position] + "!", n);
					return Error.class;
				}
			}
			else if (argument instanceof Variable) {
				Variable var = (Variable) argument;
				if (var instanceof DontCare) {
					runtime.error("Head predicate can't contain don't care variable!", n);
					return Error.class;
				}
				else if (var instanceof AggregateVariable) {
					if (!AggregateVariable.STAR.equals(var.name())) {
						Class<?> bodyType = (Class) table.current().getParent()
								.lookupLocally(var.name());
						if (bodyType == null) {
							for (Iterator<String> name = table.current().getParent().symbols();
							     name.hasNext();)
								System.err.println(name.next());
							runtime.error("Head variable " + var
									+ " not defined in rule body.", n);
							return Error.class;
						}
					}
				}
				else {
					Class<?> headType = (Class) table.current().lookupLocally(var.name());
					Class<?> bodyType = (Class) table.current().getParent().lookupLocally(var.name());
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
				jol.lang.plan.Function f =
					(jol.lang.plan.Function) node.getProperty(Constants.TYPE);
				terms.add(0, f);
			}
			else if (node.getName().equals("Predicate")) {
				Class type = (Class) dispatch(node);
				if (type == Error.class) {
					return type;
				}
				Predicate p = (Predicate) node.getProperty(Constants.TYPE);
				if (p.event() != Predicate.Event.NONE && p.notin()) {
					runtime.error("Can't apply notin to event predicate!", n);
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

		Table table = null;
		if (name.name.equals(Flatten.NAME)) {
			TypeList types = new TypeList();
			boolean valid = false;
			for (Expression arg : predicate) {
				types.add(arg.type());
				if (arg.type() == ValueList.class) {
					valid = true;
					this.table.current().define(((Variable) arg).name(), Comparable.class);
				}
			}
			table = new Flatten(jol.core.Runtime.idgen(), types);
			if (!valid) {
				runtime.warning("Flatten input schema does not contain a ValueList!", n);
			}
		}
		else {
			table = context.catalog().table(name);
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
						runtime.error("Predicate argument position " + index + " does not match table function type!", n);
						return Error.class;
					}
					index++;
				}
				if (types.length != index) {
					runtime.error("Predicate schema types must match table function types!", n);
					return Error.class;
				}
			}
		}

		n.setProperty(Constants.TYPE, new jol.lang.plan.Function(table, predicate));
		return jol.lang.plan.Function.class;
	}

	public Class visitPredicate(final GNode n) {
		boolean notin = n.getString(0) != null;
		String event = n.getString(2);

		Class type = (Class) dispatch(n.getNode(1));
		assert(type == TableName.class);
		TableName name = (TableName) n.getNode(1).getProperty(Constants.TYPE);
		if (name.scope == null) {
			name.scope = program.name();
			if (context.catalog().table(name) == null) {
				name.scope = Table.GLOBALSCOPE;
			}
		}

		/* Lookup the schema for the given tuple name. */
		Table ptable = context.catalog().table(name);
		if (ptable == null) {
			runtime.error("No catalog definition for predicate " + name, n);
			return Error.class;
		}

		Predicate.Event tableEvent = Predicate.Event.NONE;
		if (!(ptable instanceof EventTable) && event != null) {
			if ("insert".equals(event)) {
				tableEvent = Predicate.Event.INSERT;
			}
			else if ("delete".equals(event)) {
				tableEvent = Predicate.Event.DELETE;
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

		if (schema.size() < parameters.size()) {
			runtime.error("Program " + program.name() + ": Schema size mismatch on predicate " +  name, n);
			return Error.class;
		}

		List<AggregateVariable> aggVariables = new ArrayList<AggregateVariable>();
		String argLocation = null;

		/* Type check each tuple argument according to the schema. */
		for (int index = 0; index < schema.size(); index++) {
			Expression<?> param = parameters.size() <= index ?
					new DontCare(schema.get(index)) : parameters.get(index);
			Class paramType = param.type();

			if (param instanceof Cast) {
				param = ((Cast) param).expression();
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
					arguments.add(dontcare);
				}
			}

			if (param instanceof Aggregate) {
				for (AggregateVariable agg : ((Aggregate) param).variables()) {
					if (!aggVariables.contains(agg) &&
							!agg.name().equals(AggregateVariable.STAR)) {
						aggVariables.add(agg);
					}
				}
			}

			if (param instanceof Variable) {
				Variable var = (Variable) param;
				var.type(paramType);
				if (var.type() == null) {
					/* Fill in type using the schema. */
					var.type(schema.get(index));
				}

				/* Map variable to its type. */
				table.current().define(var.name(), var.type());
				paramType = var.type();

				/* We cannot have two arguments with different location specifiers */
				if (var.loc()) {
					if (argLocation == null) {
						argLocation = var.name();
					}
					else if (!argLocation.equals(var.name())) {
						runtime.error("Predicate " + name + " contains " +
									  "more than one distinct location specifier " +
									  "in its argument list: @" +
									  argLocation + ", @" + var.name(), n);
						return Error.class;
					}
				}
			}

			if (param.variables().size() > 0) {
				for (Variable var : param.variables()) {
					if (var.name().equals(AggregateVariable.STAR)) continue;
					if (!table.current().isDefined(var.name())) {
						runtime.error("Predicate " + name + " contains an argument expression based " +
									"that depends on variable " + var.name() + ", which is not" +
									"defined by some left hand side predicate!", n);
						return Error.class;
					}
				}
			}

			/* Ensure the type matches the schema definition. */
			if (!subtype(schema.get(index), paramType)) {
				if (param instanceof Value &&
						Number.class.isAssignableFrom(schema.get(index)) &&
						Number.class.isAssignableFrom(param.type())) {
					Number number = (Number) ((Value) param).value();
					try {
						Constructor<?> cons = ((Class<?>)schema.get(index)).getConstructor(String.class);
						param = new Value<Number>((Number) cons.newInstance(number.toString()));
					} catch (Exception e) {
						e.printStackTrace();
						return Error.class;
					}
				}
				else {
					runtime.error("Predicate " + name + " argument " + param.getClass() + " " + param
									+ " has type " + param.type() + " does not match type "
									+ schema.get(index) + " in schema.", n);
					return Error.class;
				}
			}
			arguments.add(param);
		}

		if (schema.size() != arguments.size()) {
			runtime.error("Schema size mismatch on predicate " +  name, n);
			return Error.class;
		}

		/* Toss all aggregate variables at the end of the tuple. */
		if (aggVariables.size() > 0) {
			arguments.addAll(aggVariables);
		}

		Predicate pred = new Predicate(context, notin, name, tableEvent, arguments);

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
					      type + " instead.", n);
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
			new jol.lang.plan.Boolean<java.lang.Boolean>(jol.lang.plan.Boolean.OR,
					ensureBooleanValue(lhs),
					ensureBooleanValue(rhs)));
		return jol.lang.plan.Boolean.class;
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
			new jol.lang.plan.Boolean(jol.lang.plan.Boolean.AND,
					ensureBooleanValue(lhs),
					ensureBooleanValue(rhs)));
		return jol.lang.plan.Boolean.class;
	}

	public Class visitEqualityExpression(final GNode n) {
		Class ltype = (Class) dispatch(n.getNode(0));
		Class rtype = (Class) dispatch(n.getNode(2));
		if (ltype == Error.class || rtype == Error.class) {
			return Error.class;
		}

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
			new jol.lang.plan.Boolean(oper,  lhs,  rhs));
		return jol.lang.plan.Boolean.class;
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
			new jol.lang.plan.Boolean(oper,  lhs,  rhs));
		return jol.lang.plan.Boolean.class;
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
				new jol.lang.plan.Boolean(jol.lang.plan.Boolean.NOT, ensureBooleanValue(expr), null));
		return jol.lang.plan.Boolean.class;
	}

	public Class visitCastExpression(final GNode n) {
		Class type = (Class) dispatch(n.getNode(0));
		if (type == Error.class) return Error.class;
		type = (Class) dispatch(n.getNode(1));
		if (type == Error.class) return Error.class;

		Class cast = (Class) n.getNode(0).getProperty(Constants.TYPE);
		Expression expr = (Expression) n.getNode(1).getProperty(Constants.TYPE);

		if (expr.type() != null && expr.type() != Comparable.class
				&& !(subtype(expr.type(), cast) || subtype(cast, expr.type()))) {
			runtime.error("CAST ERROR: Expression " + expr.toString() +
					      " type " + expr.type() + " is not a supertype of " + cast + ".", n);
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
			n.setProperty(Constants.TYPE, new jol.lang.plan.Boolean(jol.lang.plan.Boolean.IN, variable, expr));
			return jol.lang.plan.Boolean.class;
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
			n.setProperty(Constants.TYPE, new jol.lang.plan.Math(oper, lhs, rhs));
			return jol.lang.plan.Math.class;
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

		if (subtype(Number.class, lhs.type()) &&
			     subtype(Number.class, rhs.type())) {
			n.setProperty(Constants.TYPE, new jol.lang.plan.Math(oper, lhs, rhs));
			return jol.lang.plan.Math.class;
		}
		else if (subtype(String.class, lhs.type()) &&
			     subtype(String.class, rhs.type())) {
			n.setProperty(Constants.TYPE, new jol.lang.plan.Math(oper, lhs, rhs));
			return jol.lang.plan.Math.class;
		}
		else if (Set.class.isAssignableFrom(lhs.type()) &&
			     Set.class.isAssignableFrom(rhs.type())) {
			n.setProperty(Constants.TYPE, new jol.lang.plan.Math(oper, lhs, rhs));
			return jol.lang.plan.Math.class;
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
		else if (subtype(Number.class, lhs.type()) &&
			     subtype(Number.class, rhs.type())) {
			n.setProperty(Constants.TYPE, new jol.lang.plan.Math(oper, lhs, rhs));
			return jol.lang.plan.Math.class;
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
				Constructor[] constructors = newclass.type().getConstructors();
				Constructor constructor = null;
				for (Constructor c : constructors) {
					if (typeCoercion(c.getParameterTypes(), types)) {
						constructor = c;
						break;
					}
				}
				if (constructor == null) {
					runtime.error("Undefined constructor " + newclass.type() + types);
					return Error.class;
				}
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
					Method method = null;
					for (Method m : object.type().getMethods()) {
						if (m.getName().equals(reference.toString()) &&
								typeCoercion(m.getParameterTypes(), types)) {
							method = m;
							break;
						}
					}
					if (method == null) {
						runtime.error("Undefined method " + object.type() + "." + reference.toString() + types.toString());
						return Error.class;
					}

				    n.setProperty(Constants.TYPE, new MethodCall(object, method, arguments));
				    return MethodCall.class;
				}
				else {
					Method method = null;
					for (Method m : reference.type().getMethods()) {
						if (m.getName().equals(reference.toString()) &&
								typeCoercion(m.getParameterTypes(), types)) {
							method = m;
							break;
						}
					}
					if (method == null) {
						runtime.error("Undefined method " + reference.type() + "." + reference.toString() + types);
						return Error.class;
					}
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
						        " is not a variable but rather refers to the class type of " +
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
				for (Method method : type.getMethods()) {
					if (method.getName().equals(name)) {
						n.setProperty(Constants.TYPE, new UnknownReference(expr, null, name));
						return Reference.class;
					}
				}
				runtime.error("Unknown reference " + name + " in type " + type);
				return Error.class;
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
					!Modifier.isStatic(field.getModifiers())) {
					runtime.error("Field " + name + " must be static or an enumeration!", n);
					return Error.class;
				}
				n.setProperty(Constants.TYPE, new StaticReference(type, field));
				return StaticReference.class;
			} catch (Exception e) {
				/* It must be a method at this point. Punt to higher ground. */
				for (Method method : type.getMethods()) {
					if (method.getName().equals(name)) {
						n.setProperty(Constants.TYPE, new UnknownReference(null, type, name));
						return Reference.class;
					}
				}
				runtime.error("ERROR: unknown reference! " + name + " in type " + type);
				return Error.class;
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
		else if (!subtype(Number.class, expr.type())) {
			runtime.error("Expression " + expr +
					" type must be numberic in increment expression.");
			return Error.class;
		}
		n.setProperty(Constants.TYPE, new jol.lang.plan.Math(jol.lang.plan.Math.INC, expr, null));
		return jol.lang.plan.Math.class;
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
		else if  (!subtype(Number.class, expr.type())) {
			runtime.error("Expression " + expr +
					" type must be numberic in decrement expression.");
			return Error.class;
		}
		n.setProperty(Constants.TYPE, new jol.lang.plan.Math(jol.lang.plan.Math.DEC, expr, null));
		return jol.lang.plan.Math.class;
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

	//---------------------------- ValuesList ------------------------//
	public Class visitValuesList(final GNode n) {
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
		n.setProperty(Constants.TYPE, new VList(args));
		return VList.class;
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
			Class type  = null;
			String name = n.getString(0);
			if (type(name) != null) {
				type = type(name);
				name = type.getCanonicalName();
			}

			for (Object c : n.getList(1)) {
				if (type != null) {
					name  += "$" + c.toString();
					type = type(name) == null ?
							Class.forName(name) : type(name);
					NAME_TO_BASETYPE.put(name, type);
				}
				else {
					name  += "." + c.toString();
					try {
						type = Class.forName(name);
					}
					catch (ClassNotFoundException e) {
						type = null;
					}
				}
			}

			if (type == null) {
				runtime.error("Unknown class type! " + name);
				return Error.class;
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

		if ("limit".equals(function)) {
			Class type = (Class) dispatch(n.getNode(2));
			dispatch(n.getNode(1));
			if (type == Variable.class) {
				Variable kVar = (Variable) n.getNode(2).getProperty(Constants.TYPE);
				Variable var  = (Variable) n.getNode(1).getProperty(Constants.TYPE);
				n.setProperty(Constants.TYPE, new Limit(var, kVar));
			}
			else {
				Value<Number>  kConst = (Value<Number>) n.getNode(2).getProperty(Constants.TYPE);
				Variable       var    = (Variable)      n.getNode(1).getProperty(Constants.TYPE);
				n.setProperty(Constants.TYPE, new Limit(var, kConst.value()));
			}
		}
		else if ("topk".equals(function) || "bottomk".equals(function)) {
			Class type = (Class) dispatch(n.getNode(2));
			dispatch(n.getNode(1));
			if (type == Variable.class) {
				Variable kVar = (Variable) n.getNode(2).getProperty(Constants.TYPE);
				Variable var  = (Variable) n.getNode(1).getProperty(Constants.TYPE);
				if (!subtype(Number.class, kVar.type())) {
					runtime.error("Second parameter to " + function + " aggregate must be of" +
							" type Number! Variable " + var + " is type " + var.type(), n);
					return Error.class;
				}
				n.setProperty(Constants.TYPE,
						"topk".equals(function) ?
								new TopK(var, kVar) : new BottomK(var, kVar));
			}
			else {
				Value<Number>  kConst = (Value<Number>) n.getNode(2).getProperty(Constants.TYPE);
				Variable       var    = (Variable)      n.getNode(1).getProperty(Constants.TYPE);
				n.setProperty(Constants.TYPE,
						"topk".equals(function) ?
								new TopK(var, kConst.value()) :
									new BottomK(var, kConst.value()));
			}
		}
		else if ("generic".equals(function)) {
			Class type = (Class) dispatch(n.getNode(1));

			if (type != MethodCall.class) {
				runtime.error("Generic function must be a method call!!", n);
				return Error.class;
			}
			MethodCall method = (MethodCall) n.getNode(1).getProperty(Constants.TYPE);
			n.setProperty(Constants.TYPE, new GenericAggregate(method));
		}
		else {
			Class type = (Class) dispatch(n.getNode(1));
			if (type == Variable.class) {
				Variable var = (Variable) n.getNode(1).getProperty(Constants.TYPE);
				n.setProperty(Constants.TYPE,
						new Aggregate(new AggregateVariable(var), function, jol.types.function.Aggregate.type(function, var.type())));
			} else if (type == MethodCall.class) {
				MethodCall method = (MethodCall) n.getNode(1).getProperty(Constants.TYPE);
				n.setProperty(Constants.TYPE,
						new Aggregate(method, function, jol.types.function.Aggregate.type(function, method.type())));
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
		String value = n.getString(0);
		if (value.equals("infinity")) {
			n.setProperty(Constants.TYPE, new Value<Integer>(Integer.MAX_VALUE));
		}
		else  {
			n.setProperty(Constants.TYPE, new Value<Integer>(Integer.parseInt(value)));
		}
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
