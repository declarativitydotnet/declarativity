package jol.types.table;

import java.util.Hashtable;
import java.util.TimerTask;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.table.Table.Type;

public class TimerTable extends Table {
	
	private class Timer extends TimerTask implements Comparable<Timer> {
		private long delay;
		private long period;
		
		
		public Timer(long delay, long period) {
			this.delay = delay;
			this.period = period;
		}

		@Override
		public void run() {
			TupleSet timer = new TupleSet(TimerTable.this.name, new Tuple(delay, period, this));
			try {
				TimerTable.this.context.schedule(TimerTable.this.program, TimerTable.this.name, timer, null);
			} catch (UpdateException e) {
				e.printStackTrace();
				cancel();
			}
		}

		public int compareTo(Timer o) {
			return toString().compareTo(o.toString());
		}
		
		public String toString() {
			return TimerTable.this.name.toString();
		}
		
	}
	
	/** The attribute fields expected by this table function. */
	public enum Field{DELAY, PERIOD, TIMER};
	
	/** The attribute types execpted by this table function. */
	public static final Class[] SCHEMA =  {
		Long.class, // Delay
		Long.class, // Period
		Timer.class // Timer object
	};
	
	private Runtime context;
	
	private String program;
	
	private Timer timer;
	
	public TimerTable(Runtime context, String program, TableName name, long delay, long period) {
		super(name, Type.TIMER, null, new TypeList(SCHEMA));
		this.context = context;
		this.program = program;
		this.timer   = null;
	}

	@Override
	public Integer cardinality() { return 1; }

	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		if (timer != null) {
			timer.cancel();
			timer = null;
			return true;
		}
		return false;
	}

	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		Long delay  = (Long) t.value(Field.DELAY.ordinal());
		Long period = (Long) t.value(Field.PERIOD.ordinal());
		if (timer == null || delay != timer.delay || period != timer.period) {
			if (timer != null) timer.cancel();
			timer = new Timer(delay, period);
			if (period > 0) {
				context.timer().schedule(timer, delay, period);
			}
			else {
				context.timer().schedule(timer, delay);
			}
			t.value(Field.TIMER.ordinal(), timer);
			return false;
		}
		return true;
	}

	@Override
	public Index primary() { return null; }

	@Override
	public Hashtable<Key, Index> secondary() { return null; }

	@Override
	public TupleSet tuples() {
		return this.timer == null ? new TupleSet(name(), new Tuple())
		       : new TupleSet(name(), new Tuple(timer.delay, timer.period, timer));
	}
}
