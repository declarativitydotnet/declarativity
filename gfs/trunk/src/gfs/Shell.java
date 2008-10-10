package gfs;

import java.util.Arrays;
import java.util.List;

public class Shell {
    /*
     * TODO:
     *  (1) connect to an instance of P2
     *  (2) parse command-line argument into command + arguments
     *  (3) inject the appropriate inserts into P2; wait for the results to come back
     *  (4) return results to stdout
     */
    public static void main(String[] args) {
        List<String> argList = Arrays.asList(args);

        if (argList.size() == 0)
            usage();

        String op = args[0];
        argList.remove(0);

        if (op.equals("cat"))
            doConcatenate(argList);
        else if (op.equals("mkdir"))
            doCreateDir(argList);
        else if (op.equals("create"))
            doCreateFile(argList);
        else
            usage();
    }

    private static void doConcatenate(List<String> args)
    {
        if (args.size() == 0)
            usage();
    }

    private static void doCreateDir(List<String> args)
    {
        ;
    }

    private static void doCreateFile(List<String> args)
    {
        ;
    }

    private static void usage() {
        System.out.println("Usage: java gfs.Shell op_name args");
        System.out.println("Where op_name = {cat,mkdir,write}");
        System.exit(0);
    }
}
