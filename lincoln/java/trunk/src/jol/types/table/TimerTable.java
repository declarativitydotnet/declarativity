package jol.types.table;

import java.util.Map;
import java.util.TimerTask;

import jol.core.Runtime;
import jol.core.Schedule;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;

/**
 * The TimerTable. Support for both physical and logical timers.
 * Physical Timers are based on the Java {@link java.util.Timer} class. Please
 * see that documentation for further details. Logical timers are based
 * on Datalog fixpoints, and will fire at most once per such fixpoint.
 *
 * <p>
 * A timer table is created for each timer declaration made in the sytem.
 * A timer is created in the program using the following syntax:<br><br>
 * <code>
 * 		timer(<b>Name</b>, <b>Type</b>, <b>Period</b>, <b>TTL</b>, <b>Delay</b>);
 * </code> <br><br>
 * <b>Name:</b> String valued name which the timer may be referred to in a rule.<br>
 * <b>Type:</b> The timer type can be either 'logical' or 'physical'.<br>
 * <b>Period:</b> The period at which the timer should fire.<br>
 * <b>TTL:</b> The maximum number of timers that should fire.<br>
 * <b>Delay:</b> The delay that should occur before the timer begins.<br>
 * <br>
 * Depending on the timer type the values for period and delay have different
 * meanings. These values for physical timers are in units of
 * milliseconds. In the case of logical timers, these values are in units of
 * Datalog fixpoints.
 * <p>
 * A program references the timer in a rule by naming a predicate the
 * same as the timer <b>Name</b> declaration with the following predicate:
 * <code>Name(Period, TTL, Delay)</code>.
 * The Period, TTL, and Delay are the current registered values.
 * <p>
 * <code>
 * <pre>
 * Example:
 * - New logical timer that fires 5 times every 2 fixpoints with the first
 * occurring at the fixpoint immediately following the program start.
 * timer(ltimer, logical, 2, 5, 1);
 *
 * - New physical timer that fires 5 times every 2 seconds with the first
 * timer occurring 10 seconds after the program starts.
 * timer(ptimer, physical, 2000, 5, 10000);
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

	private static class PhysicalTimer extends TimerTask implements Comparable<PhysicalTimer> {
	    private Runtime context;
		private TableName name;
		private BasicTupleSet timer;

		public PhysicalTimer(Runtime context, TableName name, BasicTupleSet timer) {
		    this.context = context;
			this.name  = name;
			this.timer = timer;
		}

		@Override
		public void run() {
			try {
				this.context.schedule(name.scope, name, timer, null);
			} catch (JolRuntimeException e) {
				e.printStackTrace();
				cancel();
			}
		}

		public int compareTo(PhysicalTimer o) {
			return toString().compareTo(o.toString());
		}

		@Override
		public String toString() {
			return this.name.toString();
		}
	}

	/** The attribute fields expected by this table function. */
	public enum Field{PERIOD, TTL, DELAY};

	/** The attribute types expected by this table function. */
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
		super(name, Table.Type.TIMER, null, SCHEMA);
		this.context = context;
		this.type    = type.equalsIgnoreCase("physical") ? Type.PHYSICAL : Type.LOGICAL;
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
			this.timer = new PhysicalTimer(this.context, this.name, timer());
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

	private BasicTupleSet timer() {
		return new BasicTupleSet(new Tuple(this.period, this.ttl, this.delay));
	}

	private void reset() {
		if (this.timer != null) {
			this.timer.cancel();
			this.timer = null;
		}
	}
}
