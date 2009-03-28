package jol.lang;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;

import att.grappa.Attribute;
import att.grappa.Edge;
import att.grappa.Graph;
import att.grappa.GrappaConstants;
import att.grappa.GrappaPanel;
import att.grappa.GrappaSupport;
import att.grappa.Node;
import att.grappa.Subgraph;

import jol.lang.plan.Predicate;
import jol.types.basic.Tuple;
import jol.types.table.Aggregation;
import jol.types.table.Table;
import jol.types.table.TableName;

public class Debugger {
	
	public static Tuple tuple(String program, String debugger) {
		return new Tuple(program, debugger);
	}
	
	public static boolean toDot(String program, String definitions, String dependencies) {
		StringBuilder sb = new StringBuilder();
		sb.append("digraph " + program + "{\n");
		sb.append("\tcompound=true;\n");
		sb.append("\tranksep=1.25;\n");
		sb.append("\tlabel=\"Stratification Graph Program " + program + "\", fontsize=20;\n");
		sb.append("\tnode [shape=plaintext, fontname=\"verdana\", fontsize=16];\n");
		sb.append("\tbgcolor=white;\n");
		sb.append("\tedge [arrowsize=1.5];\n");
		sb.append(definitions);
		sb.append(dependencies);
		sb.append("\n}");

		try {
			FileWriter graphFile = new FileWriter(program + ".dot");
			graphFile.write(sb.toString().replaceAll("\\\\\"", "\""));
			graphFile.close();
		} catch (IOException e) {
			return false;
		}
		return true;
	}

	public static String edge(Predicate consumer, int consumerStratum, 
			Predicate producer, int producerStratum) {
		String edge = consumer.name().dotLabel() + " -> " + producer.name().dotLabel();
		Table table = producer.context.catalog().table(producer.name());
		if (producer.notin()) {
			if (consumerStratum <= producerStratum) {
				edge += "[color = red, label = \"notin\"]";
			}
			else {
				edge += "[label = \"notin\"]";
			}
		}
		else if (table instanceof Aggregation) {
			jol.types.table.Aggregation aggregation = (Aggregation) table;
			if (consumerStratum <= producerStratum) {
				edge += "[color = red, label = \"";
			}
			else {
				edge += "[label = \"";
			}
			List<jol.lang.plan.Aggregate> aggregates = aggregation.aggregates();
			for (jol.lang.plan.Aggregate aggregate : aggregates) {
				edge += aggregate.toString() + ", ";
			}
			edge = edge.substring(0, edge.lastIndexOf(", "));
			edge += "\"]";
		}

		return edge + ";\n";
	}


	public static boolean display(final String program, final String debugger, final Graph graph) {
		graph.setAttribute(Attribute.RANKSEP_ATTR, 1.5);
		graph.setAttribute(Attribute.NODESEP_ATTR, 1.0);
		graph.setNodeAttribute(Attribute.SHAPE_ATTR, "plaintext");
		graph.setNodeAttribute(Attribute.FONTNAME_ATTR, "verdana");
		graph.setNodeAttribute(Attribute.FONTSIZE_ATTR, 16);
		graph.setAttribute(Attribute.COLOR_ATTR, java.awt.Color.white);

		if (debugger.endsWith(".dot")) {
			try {
				File file = new File(debugger);
				file.delete();
				graph.printGraph(new FileOutputStream(file));
			} catch (FileNotFoundException e) {
				return false;
			}
		}
		else {
			Runnable plot = new Runnable() {
				public void run() {
					JFrame.setDefaultLookAndFeelDecorated(true);
					GrappaPanel panel = new GrappaPanel(graph);
					panel.setScaleToFit(false);

					String [] dotArgs = {debugger};
					try {
						Process dotProcess = Runtime.getRuntime().exec(dotArgs, null, null);
						GrappaSupport.filterGraph(graph, dotProcess);
						dotProcess.getOutputStream().close();
					}
					catch (Exception exc) {
						throw new Error (exc);
					}

					JFrame window = new JFrame("Stratification Graph Program " + program);
					JScrollPane scrolls = new JScrollPane(panel);
					window.setContentPane(scrolls);
					window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
					window.setLocation (10,30);
					window.setSize (640,480);
					window.setVisible(true);
				}
			};
			SwingUtilities.invokeLater(plot);
		}

		return true;
	}
	
	
	public static Node node(Subgraph graph, TableName name) {
		Node node = new Node(graph, name.dotLabel());
		node.setAttribute(Attribute.LABEL_ATTR, name.toString());
		return node;
	}
	
	public static Subgraph subgraph(Graph parent, Integer stratum) {
		Subgraph subgraph = new Subgraph(parent, "cluster"+stratum);
		subgraph.setAttribute(Attribute.LABEL_ATTR, "Stratum " + stratum);
		subgraph.setAttribute(Attribute.COLOR_ATTR, java.awt.Color.black);
		subgraph.setAttribute("rank", "same");
		return subgraph;
	}

	public static boolean edge(Graph graph, 
			Predicate consumer, int consumerStratum, Node consumerNode,
			Predicate producer, int producerStratum, Node producerNode) {
		Table table = producer.context.catalog().table(producer.name());
		Edge edge = new Edge(graph, consumerNode, producerNode);
		if (producer.notin()) {
			if (consumerStratum <= producerStratum) {
				edge.getAttribute(Attribute.COLOR_ATTR).setValue(java.awt.Color.red);
			}
		}
		else if (table instanceof Aggregation) {
			jol.types.table.Aggregation aggregation = (Aggregation) table;
			if (consumerStratum <= producerStratum) {
				edge.setAttribute(new Attribute(GrappaConstants.EDGE, Attribute.COLOR_ATTR, java.awt.Color.red));
			}

			String aggLabel = "";
			List<jol.lang.plan.Aggregate> aggregates = aggregation.aggregates();
			for (jol.lang.plan.Aggregate aggregate : aggregates) {
				aggLabel += aggregate.toString() + ", ";
			}
			aggLabel = aggLabel.substring(0, aggLabel.lastIndexOf(", "));
			edge.setAttribute(new Attribute(GrappaConstants.EDGE, Attribute.LABEL_ATTR, aggLabel));
		}


		return true;
	}


}
