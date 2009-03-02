package bfs;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.lang.InterruptedException;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.lang.plan.Expression;
import jol.lang.plan.Predicate;
import jol.lang.plan.Rule;
import jol.lang.plan.Term;
import jol.lang.plan.Variable;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;


public class Tap {
    public static void main(String[] args) throws JolRuntimeException, UpdateException, InterruptedException {

        String arg = args[0];
        if (arg.startsWith("-l")) {
            /* listen mode. */
            Tap n = new Tap(5678);
            n.doListen();
        } else if (arg.startsWith("-f")) {
            Tap n = new Tap(7654);
            n.doFinish();
        } else if (arg.startsWith("-r")) {
            System.out.println("ARRR: "+arg);
            String program = args[1];
            String rule = args[2];
            List<String> path = new LinkedList<String>();
            for (int i=3; i < args.length; i++) {
                System.out.println("put " + args[i] + " on stack");
                path.add(args[i]);
            }
            Tap n = new Tap(1234);
            n.doLookup(program, rule, path);
        } else {
            usage();
        }

        System.out.println("DONE\n");
    }

    private static void usage() {
        //System.err.println("Usage: bfs.Tap <olg source>, <sink>");yy
        System.err.println("Usage: bfs.Tap <opt>\n where ops is one of:\n\t-l\tlisten, or\n\t-f finish\n");
        System.exit(1);
    }

    public JolSystem system;
    public String rewrittenProgram;
    private String sink;
    private FileOutputStream watcher;
    private Map defines;
    private List<Rule> ruleList;
    private Map<String, Predicate> predHash;


    public Tap(int port) throws JolRuntimeException, UpdateException {
        this.system = Runtime.create(Runtime.DEBUG_ALL, System.err, port);
        init(sink);
    }

    public Tap(JolSystem s, String sink) {
        /* this class is being instantiated
           in a running JolSystem;
        */
        this.system = s;
        init(sink);
    }

    private void init(String sink) {
        this.sink = sink;
        this.ruleList = new LinkedList<Rule>();
        this.predHash = new HashMap<String, Predicate>();
        this.rewrittenProgram = "";
        this.defines = new HashMap();

/*
        try {
            File f = new File(sink);
            if (!f.exists()) {
                f.createNewFile();
                this.watcher = new FileOutputStream(f);
                String p = "program tap;\n";
                this.watcher.write(p.getBytes());
            } else {
                this.watcher = new FileOutputStream(f);
            }
        } catch (FileNotFoundException e) {
            System.out.println("huh?\n");
        } catch(IOException e) {
            throw new RuntimeException(e);
        }
*/
    }


    public static String join(List l, String delim, boolean quotes) {
        String ret = "";
        Iterator i = l.iterator();
        while (i.hasNext()) {
            Object current = i.next();
            String nStr = current.toString();
            if (quotes)
                nStr = "\"" + nStr + "\"";

            ret += nStr;

            if (i.hasNext()) {
                ret += delim;
            }
        }

        return ret;
    }

    public static String join(List l) {
    	return join(l, ",", false);
    }

    public static String sift(List l) {
        List<String> goods = new LinkedList<String>();

        for (Object i : l) {
            if (i.getClass() == jol.lang.plan.Assignment.class) {
                // we do not care about assignments; they are not preconditions, but
                // merely projection (that often use side-effecting (ie sequences) or
                // non-deterministic (ie clocks) functions anyway.

                // we use a separate rewrite to capture the actual projections
            } else {
                String clause = i.toString();
                String res = clause.replace("BOOLEAN","").replace("MATH","").replace("@","");
                if (!res.equals("")) {
                    goods.add(res);
                }
            }
        }

        return join(goods, ",\n\t", false);
    }

    public String header() {

        String head = "program tap;\n" +
            "import jol.core.Runtime;\n" +
            "define(tap_precondition, keys(0,1,2,3), {String, String, String, Long, Integer});\n" +
            "define(tap_send, {String, String, String, String,  Long, Integer});\n" +
            "define(tap_universe, {String, String, String, String, String});\n" +
            "define(tap_chaff, {String});\n" +
            "tap_chaff(\"foo\");\n" +
            "watch(tap_precondition, a);\n" +
            "tap_send(@Sink, Program, Rule, Head,  Ts, Id) :- \n" +
            "\ttap_precondition(Program, Rule, Head,  @Ts, Id),\n" +
            rfooter();

        return head;
    }

    public String conjoin(String delim, boolean quotes, String... arg) {
        List<String> l = new LinkedList<String>();
        for (String s : arg) {
            l.add(s);
        }
        return join(l, delim, quotes);
    }

    public boolean isNetworkRule(Rule r) {
        Variable headLoc = r.head().locationVariable();
        if (headLoc != null) {
            for (Term t: r.body()) {
                if (t.getClass() == Predicate.class) {
                    Predicate p = (Predicate)t;
                    Variable pLVar = p.locationVariable();
                    if (pLVar != null) {
                        if (pLVar.toString() != headLoc.toString()) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    public String precondition(Rule rule) {
        String name = (rule.isDelete ? "delete-" : "") + rule.name;
        String body = sift(rule.body());

        /* if this is a network rule, count that separately. */

        String r =
            "public tap_precondition(" + conjoin(", ", true, rule.program, name, rule.head().name().name) + ",Ts, count<*>) :-\n\t" +
            conjoin(",\n\t", false, body, "Ts := java.lang.System.currentTimeMillis();\n\n");



        return r;
    }

    public void summarize() {
        for (Rule r : ruleList) {
            //provenance(r);
        }
    }

    public String provenance(Rule r) {
        String rewrite = "";
        /* for each element of the rule's body, create a new rule */
        for (Term t : r.body()) {
            if (t.getClass() == Predicate.class) {
                /* new rule */
                Predicate p = (Predicate) t;
                if (predHash.containsKey(p.name().name.toString())) {
                    String newName = conjoin("_", false, "prov", p.name().scope.toString(), p.name().name.toString());
                    List<Expression> l = new LinkedList<Expression>(p.arguments());
                    l.add(new Variable(null, "Provenance", String.class));
                    String mini = newName + "(" +
                        join(l, ", ", false) +
                        ") :-\n\t" +
                        p.toString() + ",\n\t" +
                        "Provenance := \"|\" + Runtime.idgen();\n";

                }
            }
        }
        return rewrite;
    }

    public String rfooter() {
        String foot = "\tSink := \"" + this.sink + "\";\n";

        return foot;
    }

    public String rewriter(String rname, Rule rule, String sink) {
        String Head1Name = rule.head().name().name;
        String name = (rule.isDelete ? "delete-" : "") + rule.name;
        String head1 = "Ts, \"" + name + "\"";
        String body1Str =  "tap_precondition(@" + head1 + ")";

        String body = sift(rule.body());

        String rule1Str = precondition(rule);
        String rule2Str ="";
        //String rule2Str = "tap_send(@Sink, " + conjoin(", ", true, rule.program, name, rule.head().name().name) + ", Ts) :-\n\t" +
        //    "tap_precondition(" + conjoin(",", true, rule.program, name, rule.head().name().name) + " , @Ts),\n" + rfooter();

        String type = isNetworkRule(rule) ? "N" : "L";
        String rule3Str = "tap_universe(@Sink, " + conjoin(", ", true, rule.program, name, rule.head().name().name, type ) + ") :-\n\ttap_chaff(@Foo),\n" + rfooter();

        //String prov = provenance(rule);


        return rule1Str + rule2Str + rule3Str;
    }

    public void doRewrite(String program) throws JolRuntimeException, UpdateException {
        doRewrite(program, null);
    }

    protected void finalize() throws IOException {
        this.watcher.close();
    }

    protected void watch() throws IOException {
        Set s = this.defines.entrySet();
        Iterator i = s.iterator();
        while (i.hasNext()) {
            Map.Entry me = (Map.Entry) i.next();
            String key = (String) me.getKey();
            this.watcher.write(key.getBytes());
        }
    }


    public void provRewriter(String pn, Predicate pred, String type) {
        System.out.println("pred " + pn + " type " + type);

        if (type.equals("E")) {
            predHash.put(pn, pred);
        }
    }

    public String trunc(String s) {
        /* maybe some day, I'll do it the not stupid way */
        String paths[] = s.split("/");
        List<String> l = new LinkedList<String>();
        for (int i = 1; i < paths.length; i++) {
            l.add(paths[i]);
        }

    
        return join(l, "/", false);
    }

    public void doLookup(String program, String ruleName, List<String> path) throws JolRuntimeException {

        Callback reportCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    String rName = (String) t.value(0);
                    Rule r = (Rule) t.value(1);
                    System.out.println(r.toString());

                }
            }
        };

        this.system.evaluate();
        for (String prog : Conf.corpus) {
            System.out.println("DO: "+prog);
            this.system.install("tap", ClassLoader.getSystemResource(prog));
            this.system.evaluate();

        }        


        TableName tblName = new TableName("tap", "tap");
        TupleSet tap = new BasicTupleSet(tblName);
        // XXX: hack
        tap.add(new Tuple(ruleName, program));
        this.system.schedule("tap", tblName, tap, null);

        Table table = this.system.catalog().table(new TableName("tap", "query"));
        table.register(reportCallback);

        //this.system.start();

        this.system.evaluate();
        this.system.evaluate();
        this.system.evaluate();
        this.system.evaluate();
    }

    public void doListen() throws JolRuntimeException{

        Callback reportCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    //String program = (String) t.value(0);

                }
            }
        };

        this.system.evaluate();
        this.system.install("tap", ClassLoader.getSystemResource("tap/listen.olg"));
        this.system.evaluate();

        /* Identify the data directory */
        //TableName tblName = new TableName("tap", "tap");
        //TupleSet datadir = new TupleSet(tblName);
        // XXX: hack
        //datadir.add(new Tuple("tcp:localhost:12345", program));
        //this.system.schedule("tap", tblName, datadir, null);

        Table table = this.system.catalog().table(new TableName("tap", "nc_perc"));
        table.register(reportCallback);

        this.system.start();
    }

    public void doFinish() throws JolRuntimeException, InterruptedException {

        Callback reportCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
				System.out.println("YO YO\n");
				for (Tuple t : tuples) {
                    //String program = (String) t.value(0);
                }

                try {
                    Thread.sleep(8000);
                } catch(Exception e) {
                    throw new RuntimeException(e);
                }
                //System.exit(1);
            }
        };

        this.system.install("tap", ClassLoader.getSystemResource("tap/tap_done.olg"));
        this.system.evaluate();
        this.system.evaluate();

        Table table = this.system.catalog().table(new TableName("tap", "networkFires"));
        table.register(reportCallback);
        this.system.start();
    }

    public void doRewrite(String program, String file) throws JolRuntimeException, UpdateException {

        final String mySink = this.sink;

        rewrittenProgram = header();
        Callback preconditionCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    String program = (String) t.value(0);
                    String ruleName = (String) t.value(1);
                    Rule rule = (Rule) t.value(2);

                    rewrittenProgram += rewriter(ruleName, rule, mySink);
                    ruleList.add(rule);
                }
            }
        };

        Callback provenanceCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    String program = (String) t.value(0);
                    Predicate pred = (Predicate) t.value(1);
                    String predName = (String) t.value(2);
                    String type = (String) t.value(3);

                    //rewrittenProgram += rewriter(ruleName, rule, mySink);
                    provRewriter(predName, pred, type);
                }
            }
        };


        this.system.evaluate();
        this.system.install("tap", ClassLoader.getSystemResource("bfs/tap.olg"));

        this.system.evaluate();

        if (file != null)
            this.system.install(program, ClassLoader.getSystemResource(file));


        this.system.evaluate();
        /* Identify the data directory */
        TableName tblName = new TableName("tap", "tap");
        TupleSet datadir = new BasicTupleSet(tblName);
        // XXX: hack
        datadir.add(new Tuple("tcp:localhost:12345", program));
        this.system.schedule("tap", tblName, datadir, null);

        Table t2 = this.system.catalog().table(new TableName("tap", "db"));
        t2.register(provenanceCallback);

        Table table = this.system.catalog().table(new TableName("tap", "rewriteRule"));
        table.register(preconditionCallback);

        this.system.evaluate();
        this.system.evaluate();
        this.system.evaluate();

       // for (int i=0; i<20001; i++)
       //     this.system.evaluate();

        //summarize();

        URL u;
        try {
            watch();
            File tmp = File.createTempFile("tap", "olg");
            /* put our program into a temp file.  wish we didn't have to do this... */
            FileOutputStream fos = new FileOutputStream(tmp);
            fos.write(rewrittenProgram.getBytes());
            fos.close();
            String path = tmp.getAbsolutePath();
            u = new URL("file", "", path);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        this.system.install("user", u);
    }
}
