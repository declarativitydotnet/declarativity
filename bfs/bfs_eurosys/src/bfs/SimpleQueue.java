package bfs;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

public class SimpleQueue<T> {
	private LinkedBlockingQueue<T> queue;

	SimpleQueue() {
		this.queue = new LinkedBlockingQueue<T>();
	}

	/**
	 * Add a new element to the tail of the queue. Shouldn't block, since
	 * unbounded queue space is available.
	 *
	 * @param obj
	 *            The element to add. Must not be null.
	 */
	public void put(T obj) {
		try {
			this.queue.put(obj);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
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
		try {
			return this.queue.poll(timeout, TimeUnit.MILLISECONDS);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	/**
	 * Remove and return the head element of the queue. Waits an unbounded
	 * length of time for an element to be inserted.
	 *
	 * @return The former head element.
	 */
	public T get() {
		try {
			return this.queue.take();
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	public void clear() {
		this.queue.clear();
	}
}
