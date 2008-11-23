package gfs;

public class Conf {
    private static String selfAddr = null;
    private static final String[] masterAry = new String[] {
        "tcp:localhost:5500",
        "tcp:localhost:5502",
        "tcp:localhost:5503"
    };

    /* NB: This must be called before installing "gfs.olg" */
    public static void setSelfAddress(String addr) {
        selfAddr = addr;
    }

    public static String getSelfAddress() {
        if (selfAddr == null)
            throw new IllegalStateException();

        return selfAddr;
    }

    public static String getMaster(int idx) {
        return masterAry[idx];
    }

    public static int getNumMasters() {
        return masterAry.length;
    }
}
