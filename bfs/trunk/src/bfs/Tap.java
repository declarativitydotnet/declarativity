package bfs;

import java.io.File;
import java.io.FileInputStream;
import java.nio.channels.FileChannel;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.Iterator;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import jol.lang.plan.Rule;
import jol.lang.plan.Term;
import jol.lang.plan.Predicate;
import jol.lang.plan.Expression;
import jol.lang.plan.Assignment;
import jol.lang.plan.Value;
import jol.lang.plan.Variable;
import jol.lang.Compiler.CompileTable;

import bfs.TapTable;

/* a work in progress.  need to check it in because my laptop might be about to die.  shouldn't break anything 
*/

public class Tap {
    public static void main(String[] args) throws JolRuntimeException, UpdateException {
        if (args.length != 3)
            usage();


        Tap n = new Tap(args[0]);
        try {
            n.do_rewrite(args[1], args[2]);
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

    public Tap(String sink) throws JolRuntimeException, UpdateException {
        this.system = Runtime.create(Runtime.DEBUG_ALL, System.err, 12345);
        //this.sink = sink;
        init(sink);
    }

    public Tap(JolSystem s, String sink) {
        /* this class is being instantiated 
           in a running JolSystem; create rewrites for 
        */
        this.system = s;
        init(sink);
    }

    private void init(String Sink) {
        this.sink = sink;
        this.rewrittenProgram = "";
    }

    public static String join (List l, boolean newlines) {
        String ret = "";
        Iterator i = l.iterator();
        while (i.hasNext()) {
            Object current = i.next();
            String clause = current.toString();

            if (current.getClass() == jol.lang.plan.Assignment.class) {
                // we do not care about assignments; they are not preconditions, but 
                // merely projection (that often use side-effecting (ie sequences) or
                // non-deterministic (ie clocks) functions anyway.

                // we use a separate rewrite to capture the actual projections

                // we do, however, need to ensure that the head is valid; 
                Assignment ass = (Assignment)current;
                ret += ass.variable().toString() + " := null";
            } else {
                ret += clause;
            }
            if (i.hasNext()) {
                ret += ", ";
                if (newlines)
                    ret += "\n\t";
            }
        }
        return ret;
    }
   
    /* 
    public static String join_types (List l) {
        return join_types(l, null);
    }
    */

    public static String join_types (List l, String... types) {
        String ret = "{";
        for (String s : types) {
            ret += s + ",";
        }
        Iterator<Expression> i = l.iterator();
        while (i.hasNext()) {
            Class type = i.next().type();
            ret += type.toString().replace("class ", "");
            if (i.hasNext()) {
                ret += ", ";
            }
        }
        ret += "}";
        return ret;
    }


    public String rewriter(String rname, Rule rule) {
        Predicate head = rule.head();
        String rule1Str;
        String rule2Str;
        String rest;
        String body = "";


        List r1args = new LinkedList(head.arguments());

        r1args.add(0, new jol.lang.plan.Variable(null, "Ts", java.lang.Long.class));
        r1args.add(1, new jol.lang.plan.Value<String>(null, "\"" + rule.name + "\""));
        rest = join(r1args, false);

        String types = join_types(r1args);

        String types2 = join_types(head.arguments(), "String", "Long", "String");
        
        String Head1Name = "p_" + rule.head().name().name;
        String Head2Name = "s_" + rule.head().name().name;
        String head1Str = Head1Name + "(" + rest + ")";
        String head2Str = Head2Name + "( @Sink," + rest + ")";
        String body1Str =  Head1Name + "(@" + rest + ")";

        body = join(rule.body(), true);
        rule1Str = "public\n" + (rule.isDelete ? "delete" : "") + head1Str + " :- \n\t" + body + ",\n\tTs := java.lang.System.currentTimeMillis(),\n\ttic();\n\n";
        rule2Str = head2Str + " :- \n\t" + body1Str + ",\n\tSink := \"" + this.sink + "\";\n\n";


        String define = "define (" + Head1Name + ", " + types +");\n";
        String define2 = "define (" + Head2Name + ", " + types2 +");\n";
        String watch = "watch("+ Head1Name +", a);\n";
        String watch2 = "watch("+ Head2Name +", a);\n";
       
        /* 
        System.out.println(define);
        System.out.println(rule1Str);
        System.out.println("\n");
        System.out.println(rule2Str);
        */
        return define + define2 + watch + watch2 + rule1Str + rule2Str;
    }

    public void do_rewrite(String program) throws JolRuntimeException, UpdateException {
        do_rewrite(program, null);
    }

    public void do_rewrite(String program, String file) throws JolRuntimeException, UpdateException {

        Callback copyCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    String program =(String) t.value(0);
                    String ruleName =(String) t.value(1);
                    Rule rule = (Rule) t.value(2);

                    rewrittenProgram += rewriter(ruleName, rule); 
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
        //this.system.start();
    
        
        String header = "program tap;\n";
        String timer = "timer(tic, logical, 1,1,1);\n";
        //System.out.println(header + timer + rewrittenProgram);

        TupleSet compilation = new TupleSet(CompileTable.TABLENAME);
        compilation.add(new Tuple(null, null, null, header + timer + rewrittenProgram, null));
        this.system.schedule("runtime", CompileTable.TABLENAME, compilation, null);
    }

}
