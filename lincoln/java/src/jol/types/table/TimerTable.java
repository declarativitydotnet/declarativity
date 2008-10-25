package jol.types.table;

import java.util.Iterator;
import java.util.Map;
import java.util.TimerTask;

import jol.core.Runtime;
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
 * 		timer(<b>Name</b>, <b>Delay</b>, <b>Period</b>);
 * </pre> <br>
 * <b>Name:</b> String valued name to which the timer maybe referred in a rule.<br>
 * <b>Delay:</b> The delay in milliseconds that the timer should begin.<br>
 * <b>Period:</b> The period at which the timer should trigger named events.<br>
 * 
 * <p>
 * A program references the timer in a rule by naming a predicate the
 * same as the timer <b>Name</b> declaration with the following schema: 
 * <code>(Delay, Period, Timer)</code>.
 * The Delay and Period are the current registered values. The Timer
 * variable is supplied the timer object which has type {@link java.util.TimerTask}.
 * <p>
 * <code>
 * <pre>
 * Example:
 * - New timer 'newlink' that fires one time 10 seconds after the program starts.
 * timer(newlink, 10000, 0); 
 * 
 * - Add a new link from node 9 to some random node when newlink fires.
 * link("9", randomNode()) :- newlink(Delay, Period, Timer);
 *
 * - Set the the period of the newlink timer to be 5 seconds if the Period is 0
 * newlink(Delay, 5000, Timer) :- newlink(Delay, Period, Timer), Period == 0;
 * 
 * - Cancel the timer (NOTE: Can insert a new Period or Delay later to restart the timer)
 * delete
 * newlink(null, null, null) :- some body;
 * </pre>
 * </code>
 */
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
		
		@Override
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
		Long delay  = (Long) t.value(Field.DELAY.ordinal());
		Long period = (Long) t.value(Field.PERIOD.ordinal());
		
		if (delay == null && period == null) {
			throw new UpdateException("Can't have both null delay and null period!");
		}
		else if (this.timer == null) {
			set(delay, period);
			return false; // Wait for timer to fire
		}
		else if (delay == null || period == null ||
				 delay  != this.timer.delay || 
				 period != this.timer.period) {
			set(delay, period);
			return false; // Wait for timer to fire
		}
		return true; // Timer fired
	}
	
	/**
	 * Set a new timer.
	 * @param delay The timer delay (null -> 0)
	 * @param period The timer period (null -> 0)
	 */
	private void set(Long delay, Long period) {
		if (this.timer != null) timer.cancel();
		if (delay == null) delay = 0L;
		if (period == null) period = 0L;
		
		this.timer = new Timer(delay, period);
		if (period > 0) {
			context.timer().schedule(timer, delay, period);
		}
		else {
			context.timer().schedule(timer, delay);
		}
	}

	@Override
	public Index primary() { return null; }

	@Override
	public Map<Key, Index> secondary() { return null; }

	@Override
	public Iterator<Tuple> tuples() {
		return (this.timer == null ? new TupleSet(name(), new Tuple())
		       : new TupleSet(name(), new Tuple(timer.delay, timer.period, timer))).iterator();
	}
}
