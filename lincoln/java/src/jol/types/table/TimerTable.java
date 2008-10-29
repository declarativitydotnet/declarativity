package jol.types.table;

import java.util.Map;
import java.util.TimerTask;

import jol.core.Runtime;
import jol.core.Schedule;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;

/**
 * The TimerTable.
 * Timers are based on the {@link java.util.Timer} class. Please
 * see that documentation for further details.
 * 
 * <p>
 * A timer table is created for each timer declaration made in the sytem.
 * A timer is created in the program using the following syntax:<br>
 * <pre>
 * 		timer(<b>Name</b>, <b>Type</b>, <b>Period</b>, <b>TTL</b>, <b>Delay</b>);
 * </pre> <br>
 * <b>Name:</b> String valued name to which the timer maybe referred in a rule.<br>
 * <b>Type:</b> The timer type can be either 'logical' or 'physical'.<br>
 * <b>Period:</b> The period at which the timer should trigger named events.<br>
 * <b>TTL:</b> The maximum number of timers that should fire.<br>
 * <b>Delay:</b> The delay in milliseconds that the timer should begin.<br>
 * 
 * <p>
 * A program references the timer in a rule by naming a predicate the
 * same as the timer <b>Name</b> declaration with the following predicate: 
 * <code>Name(Period, TTL, Delay)</code>.
 * The Period, TTL, and Delay are the current registered values. 
 * <p>
 * <code>
 * <pre>
 * Example:
 * - New logical timer that fires every 2 fixpoints up to 2 times with the first
 * occurring immediately after the program starts.
 * at a fixpoint period of 2 (every 2 fixpoints ltimer will fire).
 * 
 * timer(ltimer, logical, 2, 5, 0); 
 * 
 * - New physical timer that fires every 2 seconds up to 5 times with the first
 * timer occurring immediately after the program starts.
 * 
 * timer(ptimer, physical, 2000, 5, 0);
 * 
 * - Trigger a fire tuple when ltimer occurs.
 * fire(Period, TTL, Delay) :- 
 *      ltimer(Period, TTL, Delay);
 *      
 * - Trigger a fire tuple when ptimer occurs.
 * fire(Period, TTL, Delay) :- 
 *      ptimer(Period, TTL, Delay);
 * </pre>
 * </code>
 */
public class TimerTable extends Table {
	private enum Type {PHYSICAL, LOGICAL};
	
	private class PhysicalTimer extends TimerTask implements Comparable<PhysicalTimer> {
		private TableName name;
		private TupleSet timer;
		
		public PhysicalTimer(TableName name, TupleSet timer) {
			this.name  = name;
			this.timer = timer;
		}

		@Override
		public void run() {
			try {
				TimerTable.this.context.schedule(name.scope, name, timer, null);
			} catch (UpdateException e) {
				e.printStackTrace();
				cancel();
			}
		}

		public int compareTo(PhysicalTimer o) {
			return toString().compareTo(o.toString());
		}
		
		@Override
		public String toString() {
			return TimerTable.this.name.toString();
		}
		
	}
	
	/** The attribute fields expected by this table function. */
	public enum Field{PERIOD, TTL, DELAY};
	
	/** The attribute types execpted by this table function. */
	public static final Class[] SCHEMA =  {
		Long.class, // Period
		Long.class, // TTL
		Long.class  // Delay
	};
	
	private Runtime context;
	
	private Type type;
	
	private long period;
	
	private long ttl;
	
	private long delay;
	
	private long count;
	
	private PhysicalTimer timer;
	
	public TimerTable(Runtime context, TableName name, String type, long period, long ttl, long delay) {
		super(name, Table.Type.TIMER, null, new TypeList(SCHEMA));
		this.context = context;
		this.type    = type.toLowerCase().equals("physical") ? Type.PHYSICAL : Type.LOGICAL;
		this.period  = period;
		this.ttl     = ttl;
		this.delay   = delay;
		this.count   = -1;
		this.timer   = null;
	}

	@Override
	public Long cardinality() { return 1L; }

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
		Long period = (Long) t.value(Field.PERIOD.ordinal());
		Long ttl    = (Long) t.value(Field.TTL.ordinal());
		
		/* I allow adjustments to period and ttl */
		if (period != null && period != this.period) {
			this.period = period;
			reset();
		}
		
		if (ttl != null && ttl != this.ttl) {
			this.ttl = ttl;
			reset();
		}
		
		count++;
		
		if (type == Type.PHYSICAL) {
			if (this.timer == null) {
				schedulePhysicalTimer();
			}
			else if (this.count >= this.ttl) {
				reset();
			}
		}
		else {
			scheduleNextLogicalTimer();
		}
		return count > 0; // wait for first timer fire!
	}
	
	/**
	 * Set a new physical timer.
	 */
	private void schedulePhysicalTimer() {
		if (this.delay == 0 && this.count == 0) {
			/* Let the current (first) insertion represent the 
			 * first logical timer. */
			count++;
		}
		
		if (this.count < this.ttl) {
			this.timer = new PhysicalTimer(this.name, timer());
			long delay = this.timer == null ? this.delay : 0L;
			if (period > 0) {
				context.timer().schedule(this.timer, delay, this.period);
			}
			else {
				context.timer().schedule(this.timer, delay);
			}
		}
	}
	
	private void scheduleNextLogicalTimer() {
		if (this.delay == 0 && count == 0) {
			/* Let the current (first) insertion represent the 
			 * first logical timer. */
			count++; 
		}
		
		if (count < this.ttl) {
			Table schedule = this.context.catalog().table(Schedule.TABLENAME);
			Long nextTimer = context.clock().current();
			if (this.count == 0) nextTimer += this.delay;
			else nextTimer += this.period;
			
			try {
				schedule.force(new Tuple(nextTimer, name().scope, name(), timer(), null));
			} catch (UpdateException e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	public Index primary() { return null; }

	@Override
	public Map<Key, Index> secondary() { return null; }

	@Override
	public Iterable<Tuple> tuples() {
		return timer();
	}
	
	private TupleSet timer() {
		return new TupleSet(name(), new Tuple(this.period, this.ttl, this.delay));
	}
	
	private void reset() {
		if (this.timer != null) {
			this.timer.cancel();
			this.timer = null;
		}
	}
}
