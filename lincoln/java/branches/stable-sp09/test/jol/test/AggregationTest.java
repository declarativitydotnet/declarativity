package jol.test;

import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;

import org.junit.Test;

public class AggregationTest extends LocalTestBase {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub

	}

	@Override
	public void setup() throws JolRuntimeException, UpdateException {
		super.setup("jol/test/aggregation.olg");
	}
	@Test
	public void countTest() throws Exception {
		try {
			BasicTupleSet set = new BasicTupleSet();
	        set.add(new Tuple(1,"a"));
	        set.add(new Tuple(1,"b"));
	        set.add(new Tuple(2,"c"));
	        sys.schedule("aggregation", new TableName("aggregation", "counted"), set, null);
	        sys.evaluate();
	        int n;
	        n = 0;
	        for(Tuple t: sys.catalog().table(new TableName("aggregation", "countStar")).tuples()) {
	        	n++;
	        	if(t.value(0).equals(1)) { assert(t.value(1).equals(2)); }
	        	if(t.value(0).equals(2)) { assert(t.value(1).equals(1)); }
	        }
	        assert(n == 2);

	        n = 0;
	        for(Tuple t: sys.catalog().table(new TableName("aggregation", "countCol")).tuples()) {
	        	n++;
	        	if(t.value(0).equals(1)) { assert(t.value(1).equals(2)); }
	        	if(t.value(0).equals(2)) { assert(t.value(1).equals(1)); }
	        }
	        assert(n == 2);
		
		} catch(Exception e) {
			e.printStackTrace();
			throw e;
		}
	}
	
}
