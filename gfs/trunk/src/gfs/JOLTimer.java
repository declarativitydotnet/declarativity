package gfs;

import java.util.concurrent.SynchronousQueue;

public class JOLTimer extends Thread {
    /** Rate at which timer is checked */
    protected int m_rate = 100;
    /** Length of timeout */
    private int m_length;
    /** Time elapsed */
    private int m_elapsed;
    private SynchronousQueue queue;

    /**
     * Creates a timer of a specified length
     * @param  length  Length of time before timeout occurs
     */
    public JOLTimer(int length, SynchronousQueue q) {
        m_length = length;
        queue = q;
        m_elapsed = 0;
    }
  
    /** Resets the timer back to zero */
    public synchronized void reset() {
        m_elapsed = 0;
    }

    /** Performs timer specific code */
    public void run() {
        for (;;) {
            // Put the timer to sleep
            try {
                Thread.sleep(m_rate);
            } catch (InterruptedException ioe) {
                continue;
            }

            // Use 'synchronized' to prevent conflicts
            synchronized (this) {
                // Increment time remaining
                m_elapsed += m_rate;

                // Check to see if the time has been exceeded
                if (m_elapsed > m_length)
                    timeout();
            }
        }
    }

    // Override this to provide custom functionality
    public void timeout() {
        System.err.println("Network timeout occurred.... terminating");
        //System.exit(1);
        // Exception e  = new Exception();
        // throw e;
        try {
            this.queue.put("timeout");
            reset();
        } catch (InterruptedException ioe) {
            ;
        }
    }
}
