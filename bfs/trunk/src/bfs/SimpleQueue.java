package bfs;

import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;

public class SimpleQueue {
    private SynchronousQueue<Object> queue;

    SimpleQueue() {
        this.queue = new SynchronousQueue<Object>();
    }

    /**
     * Add a new element to the tail of the queue. Waits an unbounded length of
     * time for queue space to become available.
     *
     * @param obj
     *            The element to add. Must not be null.
     */
    public void put(Object obj) {
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
    public Object get(long timeout) {
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
    public Object get() {
        try {
            return this.queue.take();
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}
