package bfs;

import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class SimpleQueue<T> {
	// SynchronousQueue is a queue of one item, which encourages deadlock.
	private Queue<T> queue = new LinkedList<T>();
	Lock l = new ReentrantLock();
	Condition itemReady = l.newCondition();

	/**
	 * Add a new element to the tail of the queue. Waits an unbounded length of
	 * time for queue space to become available.
	 *
	 * @param obj
	 *            The element to add. Must not be null.
	 */
	synchronized public void put(T obj) {
    	l.lock();
    	this.queue.add(obj);
    	itemReady.signal();
    	l.unlock();
	}

	/**
	 * Remove and return the head element of the queue. Waits for at most
	 * <code>timeout</code> milliseconds.
	 *
	 * @param timeout
	 *            The maximum time to wait for an element, in milliseconds.
	 * @return The former head element, or <code>null</code> if a timeout
	 *         occurred.
	 */
	public T get(long timeout) {
		boolean done = false;
		long expiry = System.currentTimeMillis() + timeout;
		T ret = null;

		l.lock();

		while(!done) {
			ret = queue.poll();
			if(ret != null) { done = true; }
			else {
				long wait = expiry - System.currentTimeMillis();
				if(wait < 0) { done = true; continue; } 
				try {
					if(!itemReady.await(wait, TimeUnit.MILLISECONDS)) { done = true; }
				} catch(InterruptedException e) {}
			}
		}
		l.unlock();
		return ret;
	}

	/**
	 * Remove and return the head element of the queue. Waits an unbounded
	 * length of time for an element to be inserted.
	 *
	 * @return The former head element.
	 */
	public T get() {
		boolean done = false;

		T ret = null;

		l.lock();

		while(!done) {
			ret = queue.poll();
			if(ret != null) { 
				done = true;
			} else { 
				try {
					itemReady.await();
				} catch (InterruptedException e) { }
			}
		}
		l.unlock();
		return ret;
	}
}
