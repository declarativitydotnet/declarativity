package bfs;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.lang.plan.Expression;
import jol.lang.plan.Predicate;
import jol.lang.plan.Rule;
import jol.lang.plan.Term;
import jol.lang.plan.Variable;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;

//import bfs.TapTable;

/* a work in progress.  need to check it in because my laptop might be about to die.  shouldn't break anything
*/

public class Tap {
    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        if (args.length != 3)
            usage();

        Tap n = new Tap(args[0]);
        try {
            n.doRewrite(args[1], args[2]);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static void usage() {
        System.err.println("Usage: bfs.Tap <olg source>, <sink>");
        System.exit(1);
    }

    public JolSystem system;
    private String sink;
    public String rewrittenProgram;
    private FileOutputStream watcher;
    private HashMap defines;

    public Tap(String sink) throws JolRuntimeException, UpdateException {
        this.system = Runtime.create(Runtime.DEBUG_ALL, System.err, 12345);
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
        this.rewrittenProgram = "";
        this.defines = new HashMap();

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
                if (res != "") {
                    goods.add(res);
                }
            }
        }

        return join(goods, ",\n\t", false);
    }

    public String header() {
        String head = "program tap;\n" +
            "define(tap_precondition, {String, String, String, Long});\n" +
            "define(tap_send, {String, String, String, String, Long});\n" +
            "define(tap_universe, {String, String, String, String});\n" +
            "define(tap_chaff, {String});\n" +
            "timer(tic, logical, 1,1,1);\n" +
            "tap_chaff(\"foo\");\n\n";

        return head;
    }

    public String conjoin(String delim, boolean quotes, String... arg) {
        List<String> l = new LinkedList<String>();
        for (String s : arg) {
            l.add(s);
        }
        return join(l, delim, quotes);
    }

    public String precondition(Rule rule) {
        String name = (rule.isDelete ? "delete-" : "") + rule.name;
        String body = sift(rule.body());

        String r =
            "public tap_precondition(" + conjoin(", ", true, rule.program, name, rule.head().name().name) + ",Ts) :-\n\t" +
            conjoin(",\n\t", false, body, "Ts := java.lang.System.currentTimeMillis()", "tic();\n\n");

        return r;
    }

    public String provenance(Rule r) {
        String rewrite = "";
        /* for each element of the rule's body, create a new rule */
        for (Term t : r.body()) {
            if (t.getClass() == Predicate.class) {
                /* new rule */
                Predicate p = (Predicate) t;
                List<Expression> l = p.arguments();
                l.add(new Variable(null, "Provenance", String.class));
                String mini = p.name().toString() + "(" + 
                    join(l, ", ", false) +
                    ") :-\n\t" +
                    p.toString() + ",\n\t" +
                    "Provenance := \"|\" + Runtime.idgen();\n";
                      
                System.out.println("mini is " + mini); 
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
        String head1 = "Ts, \""+ name + "\"";
        String body1Str =  "tap_precondition(@" + head1 + ")";

        String body = sift(rule.body());

        String rule1Str = precondition(rule);
        String rule2Str = "tap_send(@Sink, " + conjoin(", ", true, rule.program, name, rule.head().name().name) + ", Ts) :-\n\t" +
            "tap_precondition(" + conjoin(",", true, rule.program, name, rule.head().name().name) + " , @Ts),\n" + rfooter();

        String rule3Str = "tap_universe(@Sink, " + conjoin(", ", true, rule.program, name, rule.head().name().name) + ") :-\n\ttap_chaff(@Foo),\n" + rfooter();

        String prov = provenance(rule);

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
            Map.Entry me = (Map.Entry)i.next();
            String key = (String)me.getKey();
            this.watcher.write(key.getBytes());
        }
    }

    public void doRewrite(String program, String file) throws JolRuntimeException, UpdateException {

        final String mySink = this.sink;

        rewrittenProgram = header();
        Callback copyCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    String program = (String) t.value(0);
                    String ruleName = (String) t.value(1);
                    Rule rule = (Rule) t.value(2);

                    rewrittenProgram += rewriter(ruleName, rule, mySink);
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
        TupleSet datadir = new TupleSet(tblName);
        datadir.add(new Tuple(Conf.getSelfAddress(), program));
        this.system.schedule("tap", tblName, datadir, null);

        Table table = this.system.catalog().table(new TableName("tap", "rewriteRule"));
        table.register(copyCallback);

        this.system.evaluate();
        this.system.evaluate();
        this.system.evaluate();

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
