/*
 * Copyright (c) 2001-2003 Regents of the University of California.
 * All rights reserved.
 *
 * See the file LICENSE included in this distribution for details.
 */

package phiviz;

import diva.canvas.*;
import diva.canvas.connector.*;
import diva.canvas.interactor.*;
import diva.canvas.toolbox.*;
import diva.gui.BasicFrame;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.io.*;
import java.math.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;

/**
 */
public class Vis {

    protected static Hashtable<String, P2Node> nodes =
        new Hashtable<String, P2Node>();

    protected double ring_radius = 200.0;
    protected double dot_radius = 3.0;

    protected BigInteger MODULUS;

    protected SelectionInteractor p2_node_interactor;

    private JCanvas canvas;
    private GraphicsPane graphicsPane;

    protected JSlider zoom_slider;
    protected double zoom;

    protected JScrollBar x_scrollbar, y_scrollbar;
    protected double x_offset, y_offset;

    protected JLabel node_count_label;
    protected BasicFrame frame;

    protected BasicEllipse the_ring;

    protected long check_period_ms = 5*60*1000;

    private final double decimal (BigInteger guid) {
      return (new BigDecimal(guid).divide(new BigDecimal(MODULUS),
                                          6 /* scale */,
                                          BigDecimal.ROUND_UP)).doubleValue ();
    }

    private final double theta (BigInteger guid) {
        return 2 * Math.PI * ((decimal (guid)) + 0.75);
    }

    public final double x_pos_g (BigInteger guid) {
        return Math.cos (theta (guid)) * ring_radius;
    }

    public final double y_pos_g (BigInteger guid) {
        return Math.sin (theta (guid)) * ring_radius;
    }

    public class P2Node {
    	public static final int LEAF_SET_SIZE = 1;
    	public HOST host;
        public String guid_str;
        public BigInteger guid;
        private BasicFigure dot;
        private BasicFigure token = null;
        private LinkedList ls_connectors = new LinkedList ();
        private LinkedList rt_connectors = new LinkedList ();
        private LabelFigure hostname_label;
        private boolean show_leaf_set, show_hostname, show_rt;

        private final double decimal () {
            assert guid != null;
            return (new BigDecimal(guid).divide(new BigDecimal(MODULUS),
                                                6, BigDecimal.ROUND_UP)).
                doubleValue ();
        }

        private final double theta () {
            return 2 * Math.PI * ((decimal ()) + 0.75);
        }

        public final double x_pos () {
            return Math.cos (theta ()) * ring_radius - radius ();
        }

        public final double y_pos () {
            return Math.sin (theta ()) * ring_radius - radius ();
        }

        public final double radius () {
            return dot_radius;
        }

        public final double diameter () {
            return 2.0*radius ();
        }

        public boolean in_range_mod (
                BigInteger low, BigInteger high, BigInteger query, BigInteger mod) {
            if (low.compareTo (high) <= 0) {
                // [low, high] does not wrap around.
            }
            else {
                // [low, high] wraps; make it not.
                high = high.add (mod);
            }

            if (low.compareTo (query) <= 0) {
                // [low, query] does not wrap around.
            }
            else {
                // [low, query] wraps; make it not.
                query = query.add (mod);
            }

            boolean result = ((query.compareTo (high) <= 0) &&
                    (query.compareTo (low) >= 0));
            return result;
        }

        public ArcConnector make_arc(P2Node other,
                                     boolean exterior,
                                     Color color) {
          ConnectorTarget target = new PerimeterTarget();
          Site a = null, b = null;
          boolean succ = in_range_mod (guid,
                                       guid.add(MODULUS.
                                                divide(BigInteger.valueOf(2))),
                                       other.guid, MODULUS);
          if (succ) {
            a = target.getTailSite (exterior ? other.dot : dot, 0.0, 0.0);
            b = target.getHeadSite (exterior ? dot : other.dot, 0.0, 0.0);
          }
          else {
            a = target.getTailSite (exterior ? dot : other.dot, 0.0, 0.0);
            b = target.getHeadSite (exterior ? other.dot : dot, 0.0, 0.0);
          }
          ArcConnector conn = new ArcConnector (a, b);
          Rectangle2D.Double bounds =
              (Rectangle2D.Double) conn.getBounds ();
          double len = Math.sqrt (bounds.height * bounds.height +
                                  bounds.width * bounds.width) / 4.0;
          Arrowhead arrow =
              new Arrowhead(b.getX(), b.getY(), b.getNormal());
          arrow.setLength (Math.min (len, dot_radius * 3));
          if (succ) {
            if (exterior)
              conn.setTailEnd(arrow);
            else
              conn.setHeadEnd(arrow);
          }
          else {
            if (exterior)
              conn.setHeadEnd(arrow);
            else
              conn.setTailEnd(arrow);
          }
          conn.setStrokePaint (color);
          return conn;
        }

        public void redraw() {
          FigureLayer layer = graphicsPane.getForegroundLayer();
          if (layer.contains (dot)) {
            layer.remove(dot);
          }
          if (token != null) {
            if (layer.contains(token)) {
              layer.remove(token);
            }
          }
          if (PhiVizApplication.TOKEN.contains(host)) {
            token = new BasicEllipse(x_pos()-5, y_pos()-5,
                diameter()+10, diameter()+10,
                Color.red);
            layer.add(token);
          }
          dot = new BasicEllipse(x_pos (), y_pos (),
                                 diameter (),
                                 diameter (),
                                 (host == null) ? Color.black : Color.blue);
          dot.setUserObject (this);
          dot.setInteractor (p2_node_interactor);
          layer.add (dot);
          redraw_leaf_set ();
          redraw_rt ();
          redraw_hostname ();
        }

        public P2Node(String g, HOST h) {
          show_hostname = true;
          show_leaf_set = true;
          show_rt = false;
          guid_str = g;
          if (g.substring(0,2).equalsIgnoreCase("0x"))
            g = g.substring(2, g.length()-1);
          guid = new BigInteger(g, 16);
          host = h;
          if (host != null)
            host.setGUID(guid_str);
          //redraw (); Must use callbacks to do this
        }

        public void remove () {
          hide_leaf_set ();
          hide_rt ();
          hide_hostname ();
          FigureLayer layer = graphicsPane.getForegroundLayer();
          layer.remove (dot);
        }

        protected void undraw_leaf_set () {
          FigureLayer layer = graphicsPane.getForegroundLayer();
          while (! ls_connectors.isEmpty ()) {
            Figure conn = (Figure) ls_connectors.removeFirst ();
            layer.remove (conn);
          }
        }

        public void redraw_leaf_set () {
          undraw_leaf_set ();

          if (! show_leaf_set || host == null)
            return;
          for (Iterator iter = host.successors().iterator();
               iter.hasNext();
              ) {
            HOST h = (HOST) iter.next();
            String g = h.getGUID();
            if (g == null || !nodes.containsKey(g)) continue;
            FigureLayer layer = graphicsPane.getForegroundLayer();
            ArcConnector conn = make_arc ((P2Node) nodes.get(g), true,
                                          Color.green);
            layer.add (conn);
            ls_connectors.add (conn);
          }
        }

        public void show_leaf_set () {
          show_leaf_set = true;
          redraw_leaf_set ();
        }

        public void hide_leaf_set () {
          show_leaf_set = false;
          undraw_leaf_set ();
        }

        protected void undraw_rt() {
          FigureLayer layer = graphicsPane.getForegroundLayer();
          while (! rt_connectors.isEmpty ()) {
            Figure conn = (Figure) rt_connectors.removeFirst ();
            layer.remove (conn);
          }
        }

        public void redraw_rt() {
            undraw_rt ();
            if (! show_rt || host == null)
              return;
            FigureLayer layer = graphicsPane.getForegroundLayer();
            ConnectorTarget target = new PerimeterTarget();

            Iterator j = host.fingers().iterator ();
            for (int i = 0; j.hasNext (); i++) {
              P2Node other = (P2Node) nodes.get(((HOST) j.next ()).getGUID());
              if (other == null) continue;
              Site a = target.getTailSite (dot, 0.0, 0.0);
              Site b = target.getHeadSite (other.dot, 0.0, 0.0);
              StraightConnector conn = new StraightConnector (a, b);
              Rectangle2D.Double bounds =
                  (Rectangle2D.Double) conn.getBounds ();
              double len = Math.sqrt (bounds.height * bounds.height +
                                      bounds.width * bounds.width) / 4.0;
              Arrowhead arrow =
                  new Arrowhead(b.getX(), b.getY(), b.getNormal());
              arrow.setLength (Math.min (len, dot_radius * 3));
              conn.setHeadEnd(arrow);
              conn.setStrokePaint (Color.red);
              layer.add (conn);
              rt_connectors.add (conn);
            }

            PathFigure pf =
                new PathFigure(new Line2D.Double(0,
                                                 ring_radius,
                                                 0,
                                                 -1.0*ring_radius));
            pf.setDashArray (new float [] {(float) 5.0, (float) 5.0});
            layer.add (pf);
            rt_connectors.add (pf);
            for (int i = 2; i < 32; i*=2) {
              BigInteger dist = MODULUS.divide (BigInteger.valueOf (i));
              BigInteger which = guid.divide (dist);
              BigInteger dest = which.multiply (dist).add (dist.divide
                  (BigInteger.valueOf (2)));
              pf = new PathFigure (new Line2D.Double (0, 0,
                  x_pos_g (dest), y_pos_g (dest)));
              pf.setDashArray (new float [] {(float) 5.0, (float) 5.0});
              layer.add (pf);
              rt_connectors.add (pf);
            }
          }

          public void show_rt () {
            show_rt = true;
            redraw_rt ();
          }

          public void hide_rt () {
            show_rt = false;
            undraw_rt ();
          }

          public int hostname_anchors [] = {
              SwingConstants.SOUTH,
              SwingConstants.SOUTH_WEST,
              SwingConstants.WEST,
              SwingConstants.NORTH_WEST,
              SwingConstants.NORTH,
              SwingConstants.NORTH_EAST,
              SwingConstants.EAST,
              SwingConstants.SOUTH_EAST
          };

          public void undraw_hostname () {
            if (hostname_label != null) {
              FigureLayer layer = graphicsPane.getForegroundLayer();
              layer.remove (hostname_label);
            }
            hostname_label = null;
          }

          public void redraw_hostname () {
            undraw_hostname ();
            if (! show_hostname || host == null)
              return;

            int pie = (int) ((decimal () + 1.0/16.0) * 8);
            if (pie >= hostname_anchors.length)
              pie = 0;
            hostname_label = new LabelFigure (host.getNAME());
            hostname_label.setAnchor (hostname_anchors [pie]);
            Point2D pt =
                CanvasUtilities.
                getLocation(dot.getBounds (),
                            CanvasUtilities.
                            reverseDirection(hostname_anchors[pie]));
            hostname_label.translateTo (pt);
            FigureLayer layer = graphicsPane.getForegroundLayer();
            layer.add (hostname_label);
        }

        public void show_hostname () {
            show_hostname = true;
            redraw_hostname ();
        }

        public void hide_hostname () {
            show_hostname = false;
            redraw_hostname ();
        }

        public void color_node (Color C) {
            dot.setFillPaint (C);
        }

        public void color_node_default () {
            dot.setFillPaint (Color.blue);
        }
    }

    public void new_node (String g, HOST h) {
    	if (!nodes.containsKey(g)) {
    		P2Node new_node = new P2Node (g, h);
    		nodes.put(g, new_node);
    	}
    }

    protected void fit_in_window () {

        // Find the bounding box for all points in the figure.

        FigureLayer layer = graphicsPane.getForegroundLayer();
        Rectangle2D allpoints = null;
        for (Iterator i = layer.figures (); i.hasNext (); ) {
            Figure f = (Figure) i.next ();
            if (allpoints == null)
                allpoints = f.getBounds ();
            else
                allpoints.add (f.getBounds());
        }

        // Move scrollbars to represent center of points

        x_scrollbar.setValue ((int)
                              Math.round (allpoints.getX () + 0.5*allpoints.getWidth ()));
        y_scrollbar.setValue ((int)
                              Math.round (allpoints.getY () + 0.5*allpoints.getHeight ()));

        // Adjust zoom so all points fit with a bit of space to spare

        Dimension2D d = (Dimension2D) canvas.getSize ();
        double xscale = d.getWidth () / allpoints.getWidth ();
        double yscale = d.getHeight () / allpoints.getHeight ();
        double desired_zoom = Math.min (xscale, yscale) * 0.8;
        zoom_slider.setValue (zoom_to_slider (desired_zoom));
    }

    protected void update_transform () {
        CanvasPane pane = canvas.getCanvasPane();
        Dimension2D d = (Dimension2D) canvas.getSize ();
        AffineTransform t = new AffineTransform();

        // Note: transforms are applied backwards.

        // Last, translate origin to center of frame.
        t.translate (0.5 * d.getWidth (), 0.5 * d.getHeight ());

        // Then, account for zoom.
        t.scale (zoom, zoom);

        // First, account for scrollbars.
        t.translate (x_offset, y_offset);

        pane.setTransform (t);
        // canvas.invalidate();
        pane.repaint();
    }

    public Vis () {
    }

    public Vis (BigInteger mod) throws IOException{
        MODULUS = mod;
        JPanel xpanel = new JPanel ();
        xpanel.setOpaque (true);
        xpanel.setLayout (new BoxLayout (xpanel, BoxLayout.X_AXIS));

        canvas = new JCanvas ();
        graphicsPane = (GraphicsPane) canvas.getCanvasPane ();
        xpanel.add (canvas);

        y_offset = 0.0;
        y_scrollbar = new JScrollBar (SwingConstants.VERTICAL,
                                      0, 100, -300, 300);
        y_scrollbar.setBlockIncrement(60);
        y_scrollbar.setUnitIncrement(10);

        y_scrollbar.addAdjustmentListener (new AdjustmentListener () {
                public void adjustmentValueChanged (AdjustmentEvent e) {
                    y_offset = -1.0 * y_scrollbar.getValue ();
                    update_transform ();
                }
	    });
        xpanel.add (y_scrollbar);

        JPanel panel = new JPanel ();
        panel.setOpaque (true);
        panel.setLayout (new BoxLayout (panel, BoxLayout.Y_AXIS));
        panel.add (xpanel);

        x_offset = 0.0;
        x_scrollbar = new JScrollBar (SwingConstants.HORIZONTAL,
                                      0, 100, -300, 300);
        x_scrollbar.setBlockIncrement(60);
        x_scrollbar.setUnitIncrement(10);

        x_scrollbar.addAdjustmentListener (new AdjustmentListener () {
                public void adjustmentValueChanged (AdjustmentEvent e) {
                    x_offset = -1.0 * x_scrollbar.getValue ();
                    update_transform ();
                }
	    });
        panel.add (x_scrollbar);

        JPanel count_and_scroll = new JPanel ();
        count_and_scroll.setOpaque (true);
        count_and_scroll.setLayout (new BoxLayout(count_and_scroll,
                                                  BoxLayout.X_AXIS));

        count_and_scroll.add (new JLabel ("     "));

        node_count_label = new JLabel ("");
        update_node_count ();

        count_and_scroll.add (node_count_label);

        count_and_scroll.add (new JLabel ("     "));

        count_and_scroll.add (new JLabel ("Zoom:"));

        count_and_scroll.add (new JLabel ("     "));

        zoom = 1.0;
        zoom_slider = new JSlider (SwingConstants.HORIZONTAL, -100, 100, 0);
        zoom_slider.addChangeListener (new ChangeListener () {
          public void stateChanged (ChangeEvent e) {
                    zoom = slider_to_zoom (zoom_slider.getValue ());
                    update_transform ();
                }
	    });
        count_and_scroll.add (zoom_slider);

        count_and_scroll.add (new JLabel ("     "));

        panel.add (count_and_scroll);

        p2_node_interactor = new SelectionInteractor ();
        SelectionDragger dragger = new SelectionDragger (graphicsPane);
        dragger.addSelectionInteractor (p2_node_interactor);

        frame = new BasicFrame ("P2 Chord Visualizer");
        frame.setSize (500, 400);

        frame.getContentPane ().setLayout (new BorderLayout ());
        frame.getContentPane ().add (panel);

        create_menus (frame);

        FigureLayer layer = graphicsPane.getForegroundLayer();
        the_ring = new BasicEllipse( -1.0*ring_radius, -1.0*ring_radius,
                                      2.0*ring_radius, 2.0*ring_radius);
        layer.add (the_ring);
        new_node("0x0I", null);

        update_transform ();

        frame.setVisible (true);
    }

    protected double slider_to_zoom (int slider) {
        return Math.pow (10.0, zoom_slider.getValue () / 100.0);
    }

    protected int zoom_to_slider (double zoom) {
        // zoom = 10^(slider/100)
        // log (zoom) = slider/100
        // slider = 100*log (zoom)
        return (int) Math.round (100.0 * Math.log (zoom) / Math.log (10.0));
    }

    protected void create_menus (BasicFrame frame) {
        JMenuBar menu_bar = frame.getJMenuBar ();

        JMenu menu = menu_bar.getMenu(0);

        JMenuItem item;
        item = new JMenuItem ("Change Dot Radius");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    change_dot_radius_dialog ();
                }
	    });

        item = new JMenuItem ("Reset Node Colors");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    reset_node_colors ();
                }
	    });

        menu.addSeparator ();

        item = new JMenuItem ("Show Hostname");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    for_all_nodes (new ForNodeFn () {
                            public void for_node (P2Node node) {
                                node.show_hostname ();
                            }
			});
                }
	    });

        item = new JMenuItem ("Hide Hostname");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    for_all_nodes (new ForNodeFn () {
                            public void for_node (P2Node node) {
                                node.hide_hostname ();
                            }
			});
                }
	    });

        menu.addSeparator ();

        item = new JMenuItem ("Show leaf sets");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    for_all_nodes (new ForNodeFn () {
                            public void for_node (P2Node node) {
                                node.show_leaf_set ();
                            }
			});
                }
	    });

        item = new JMenuItem ("Hide leaf sets");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    for_all_nodes (new ForNodeFn () {
                            public void for_node (P2Node node) {
                                node.hide_leaf_set ();
                            }
			});
                }
	    });

        menu.addSeparator ();

        item = new JMenuItem ("Show routing tables");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    for_all_nodes (new ForNodeFn () {
                            public void for_node (P2Node node) {
                                node.show_rt ();
                            }
			});
                }
	    });

        item = new JMenuItem ("Hide routing tables");
        menu.add (item);
        item.addActionListener (new ActionListener () {
                public void actionPerformed(ActionEvent e) {
                    for_all_nodes (new ForNodeFn () {
                            public void for_node (P2Node node) {
                                node.hide_rt ();
                            }
			});
                }
	    });
    }

    protected void change_dot_radius_dialog () {
      String s =
          (String) JOptionPane.
          showInputDialog(frame, "Change dot radius:", "Change Dot Radius",
                          JOptionPane.PLAIN_MESSAGE,
                          null, null, Double.toString (dot_radius));

      if (s != null) {
        double value = Double.parseDouble (s);
        set_dot_radius (value);
      }
    }

    protected void reset_node_colors () {
        P2Node current_node;
        Iterator IT = nodes.values().iterator();

        while (IT.hasNext ()) {
            current_node = (P2Node) IT.next ();
            current_node.color_node_default ();
            current_node.hide_hostname ();
        }
    }

    protected void update_node_count () {
        node_count_label.setText ("nodes up: " + nodes.size ());
        node_count_label.repaint ();
    }

    protected void set_dot_radius (double value) {
        dot_radius = value;
        redraw_all ();
    }

    protected void redraw_all () {
      FigureLayer layer = graphicsPane.getForegroundLayer();
      if (the_ring == null) {
        the_ring =
            new BasicEllipse(-1.0*ring_radius, -1.0*ring_radius,
                             2.0*ring_radius, 2.0*ring_radius);
        the_ring.setLineWidth ((float) (dot_radius/3.0));
        the_ring.repaint ();
        layer.add (the_ring);
      }
      for (P2Node b : nodes.values ()) {
        b.redraw();
      }
    }

    protected static interface ForNodeFn {
        void for_node (P2Node node);
    }

    public void for_all_nodes (ForNodeFn fn) {
        SelectionModel model = p2_node_interactor.getSelectionModel();
        Iterator i = model.getSelection ();
        while (i.hasNext ()) {
            Figure dot = (Figure) i.next ();
            if (dot.getUserObject () instanceof P2Node) {
                P2Node node = (P2Node) dot.getUserObject ();
                fn.for_node (node);
            }
        }
    }

    public void resize_scrollbars () {
        FigureLayer layer = graphicsPane.getForegroundLayer();
        Rectangle2D allpoints = null;
        for (Iterator i = layer.figures (); i.hasNext (); ) {
            Figure f = (Figure) i.next ();
            if (allpoints == null)
                allpoints = f.getBounds ();
            else
                allpoints.add (f.getBounds());
        }

        double max_x_coor = allpoints.getX () + allpoints.getWidth ();
        double min_x_coor = allpoints.getX ();
        double max_y_coor = allpoints.getY () + allpoints.getHeight ();
        double min_y_coor = allpoints.getY ();

        y_scrollbar.setValues((int) Math.round (allpoints.getY () +
                                                0.5*allpoints.getHeight ()),
                              (int) Math.round (0.25 * allpoints.getHeight ()),
                              (int) Math.round ((min_y_coor) -
                                                Math.abs (min_y_coor * 0.01)),
                              (int) Math.round ((max_y_coor) +
                                                Math.abs (max_y_coor * 0.01)));

        x_scrollbar.setValues((int) Math.round (allpoints.getX () +
                                                0.5*allpoints.getWidth ()),
                              (int) Math.round (0.25 * allpoints.getWidth ()),
                              (int) Math.round ((min_x_coor) -
                                                Math.abs (min_x_coor * 0.01)),
                              (int) Math.round ((max_x_coor) +
                                                Math.abs (max_x_coor * 0.01)));

        int x_block_increment = (int) Math.round ((max_x_coor -
                                                   min_x_coor) / 10);
        x_scrollbar.setBlockIncrement ((x_block_increment > 10) ?
                                       x_block_increment : 10);
        x_scrollbar.setUnitIncrement ((x_block_increment > 40) ?
                                      (x_block_increment / 40) : 1);

        int y_block_increment = (int) Math.round ((max_y_coor -
                                                   min_y_coor) / 10);
        y_scrollbar.setBlockIncrement ((y_block_increment > 10) ?
                                       y_block_increment : 10);
        y_scrollbar.setUnitIncrement ((y_block_increment > 40) ?
                                      (y_block_increment / 40) : 1);

        Dimension2D d = (Dimension2D) canvas.getSize ();
        double xscale = d.getWidth () / allpoints.getWidth ();
        double yscale = d.getHeight () / allpoints.getHeight ();
        double desired_zoom = Math.min (xscale, yscale) * 0.8;
        zoom_slider.setValue (zoom_to_slider (desired_zoom));
    }

    public void redraw () {
    	redraw_all ();
    	update_transform();
    }

    public static void main (String [] args) throws IOException {
        Vis vis = new Vis (BigInteger.valueOf (2).pow (160));
        vis.redraw ();
    }
}
