package jol.types.table;

import java.util.ArrayList;
import java.util.List;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;

/**
 * Flatten function will flatten all attributes
 * of type ValueList in the input tuple set.
 *
 * <p>
 * For each tuple in the input, flatten will perform
 * a cross product of all ValueList attributes using
 * the list base values. For example: <br>
 *
 * <pre>
 * <code>
 * input  tupleset: <1, ["foo", "bar"], [1.0, 2.0]>
 *
 * output tupleset: <1, "foo", 1.0>, <1, "bar", 1.0>,
 *                  <1, "foo", 2.0>, <1, "bar", 2.0>,
 * </code>
 * </pre>
 *
 * <p>
 * The type of the flatten output will be #Comparable.
 * A cast must be made to the base type before inserting into
 * a table if that table accepts a flatten attribute and is
 * a type other than #Comparable. <br>
 * For example (taken from test/path.olg): <br>
 *
 * <pre>
 * <code>
 * define(allpaths, {String, ValueList});
 * define(flatpaths, {String, Integer});
 *
 * flatpaths(Source, Path) :-
 *    flatten(allpaths(Source, Paths)),
 *    Path := (Integer) Paths; // Need explicit cast to Integer
 *
 * // NOTE: The following is not supported and will invoke a type check error!
 * flatpaths(Source, (Integer) Paths) :-
 * 	  flatten(allpaths(Source, Paths));
 *
 * </code>
 * </pred>
 */
public class Flatten extends Function {
	public static final String NAME = "flatten";

	private List<Integer> positions;

	public Flatten(Long id, Class[] inputTypes) {
		super(NAME + ":" + id, inputTypes);
		this.positions = new ArrayList<Integer>();
		Class[] outputTypes = new Class[inputTypes.length];
		for (int i = 0; i < inputTypes.length; i++) {
			Class input = inputTypes[i];
			if (List.class.isAssignableFrom(input)) {
				outputTypes[i] = Object.class;
				this.positions.add(i);
			}
			else {
				outputTypes[i] = input;
			}
		}
		this.attributeTypes = outputTypes;
	}

	@Override
	public TupleSet insert(TupleSet tuples, TupleSet conflicts)
			throws UpdateException {
		if (this.positions.size() == 0) {
			return tuples;
		}
		else {
			return flatten(tuples, new ArrayList<Integer>(this.positions));
		}
	}

	/**
	 * @param tuples
	 * @param positions
	 * @return
	 */
	private TupleSet flatten(TupleSet tuples, List<Integer> flattenPositions) {
		if (flattenPositions.size() == 0) {
			return tuples;
		}
		else {
			/* Flatten along the first position */
			TupleSet delta = new TupleSet(name());
			int   position = flattenPositions.remove(0);
			for (Tuple tuple : tuples) {
				List<Object> list = (List<Object>) tuple.value(position);
				if (list != null) {
					for (Object value : list) {
						Object flattened[] = tuple.toArray();
						//Tuple flattened = tuple.clone();
						flattened[position] = value; //.value(position, value);
						delta.add(new Tuple(flattened));
					}
				}
			}
			/* Recursively flatten along remaining positions. */
			return flatten(delta, flattenPositions);
		}
	}

}
