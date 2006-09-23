package phiviz;

import java.io.*;
import java.util.*;
import javax.imageio.*;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

/**
 * <p>Title: PhiViz</p>
 * <p>Description: Phi Visualization</p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: Intel Corporation</p>
 * @author Petros Maniatis
 * @version 1.0
 */

public class PhiVizFrame
    extends JFrame {
  PhiPanel phiPanel;
  JPanel contentPane;
  JScrollPane phiScroller;

  /**
   * The main selection. Changed every time the selection changes while the main
   * selection is the current one.
   */
  private Set mainSelected_ = new HashSet();

  /**
   * The alternate selection.  Changed every time the selection changes while
   * the main selection is NOT the current one.
   */
  private Set altSelected_ = new HashSet();


  /**
   * The access structure to the PlanetLab indices.
   */
  private PlanetLabHelper planetLabHelper_;

  Border contentPaneBorder;
  GridBagLayout contentPaneGridBagLayout = new GridBagLayout();

  /**
   * The application icon image file name, from the working directory.
   */
  public static final String ICON_IMAGE_FILE_NAME = "phiVizIcon.jpg";

  /**
   * The application icon image.
   */
  private static final Image phiIconImage;
  static {
    try {
      phiIconImage = ImageIO.read(new FileInputStream(ICON_IMAGE_FILE_NAME));
    }
    catch (IOException ex) {
      throw new RuntimeException("Could not load application icon " +
                                 ICON_IMAGE_FILE_NAME);
    }
  }

  JPanel controlPanel = new JPanel();
  GridBagLayout controlGridBagLayout = new GridBagLayout();

  /**
   * The length of the square drawn around every site.
   */
  public static final int SIDE = 8;

  //Construct the frame
  public PhiVizFrame(BufferedImage mapImage, PlanetLabHelper sites) {
    if (sites == null) {
      throw new NullPointerException("PHI Node helper cannot be null");
    }
    this.planetLabHelper_ = sites;

    enableEvents(AWTEvent.WINDOW_EVENT_MASK);
    try {
      jbInit(mapImage);
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }

  public PhiPanel panel() {
	  return phiPanel;
  }

  public void update() {
	  phiPanel.update();
  }

  //Component initialization
  private void jbInit(BufferedImage mapImage)
      throws Exception  {
    phiPanel = new PhiPanel(mapImage, planetLabHelper_);
    jbInit();
    setIconImage(phiIconImage);
    phiScroller.setMaximumSize(phiPanel.getPreferredSize());
  }


  //Overridden so we can exit when window is closed
  protected void processWindowEvent(WindowEvent e) {
    super.processWindowEvent(e);
    if (e.getID() == WindowEvent.WINDOW_CLOSING) {
      System.exit(0);
    }
  }

  private void jbInit() throws Exception {
	JMenuBar menuBar = new JMenuBar();
	JMenu menu = new JMenu("Display");
	menuBar.add(menu);
	this.setJMenuBar(menuBar);

    contentPane = (JPanel) this.getContentPane();
    contentPaneBorder = BorderFactory.createEtchedBorder(Color.white,new Color(165, 163, 151));
    contentPane.setLayout(contentPaneGridBagLayout);
    this.setSize(new Dimension(495, 345));
    this.setTitle("P2 PlanetLab Visualizer");
    phiScroller = new JScrollPane(phiPanel);
    phiScroller.setViewportBorder(contentPaneBorder);
    phiScroller.getViewport().setBackground(Color.orange);
    phiScroller.setAlignmentX((float) 0.0);
    phiScroller.setAlignmentY((float) 0.0);
    phiScroller.setAutoscrolls(true);
    contentPane.setBackground(Color.green);
    contentPane.setAlignmentX((float) 0.0);
    contentPane.setAlignmentY((float) 0.0);
    contentPane.setBorder(contentPaneBorder);
    phiPanel.addMouseListener(new PhiVizFrame_phiPanel_mouseAdapter(this));
    phiPanel.setBackground(Color.red);
    phiPanel.addMouseMotionListener(new PhiVizFrame_phiPanel_mouseMotionAdapter(this));
    zoomLabel.setText("Zoom:");
    zoomLabel.setVerticalTextPosition(javax.swing.SwingConstants.CENTER);
    zoomLabel.setToolTipText("");
    zoomLabel.setVerifyInputWhenFocusTarget(true);
    zoomLabel.setHorizontalAlignment(SwingConstants.TRAILING);
    zoomLabel.setHorizontalTextPosition(SwingConstants.TRAILING);
    zoomLabel.setFont(new java.awt.Font("Tahoma", 0, 11));
    zoomLabel.setAlignmentX((float) 0.5);
    zoomSlider.setOrientation(JSlider.HORIZONTAL);
    zoomSlider.setMaximum(10);
    zoomSlider.setMinimum(-10);
    zoomSlider.setToolTipText("Slide to zoom the map in or out");
    zoomSlider.addChangeListener(new PhiVizFrame_zoomSlider_changeAdapter(this));
    zoomSlider.setValue(0);
    mapPanel.setLayout(mapGridBagLayout1);
    mapPanel.setBorder(BorderFactory.createEtchedBorder());
    mapPanel.setDebugGraphicsOptions(0);
    mapPanel.setMaximumSize(new Dimension(32767, 32767));
    zoomPanel.setLayout(gridBagLayout1);
    errorsLabel.setBackground(new Color(100, 255, 100));
    errorsLabel.setText("E:");
    errorsField.setColumns(6);
    errorsField.setText("");
    errorsField.setBackground(new Color(255, 100, 100));
    errorsField.setToolTipText("Errors received");
    errorsField.setCaretPosition(0);
    errorsField.setEditable(false);
    contentPane.add(controlPanel,          new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        contentPane.add(mapPanel,               new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    mapPanel.add(phiScroller,    new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 400, 200));
    mapPanel.add(zoomPanel, new GridBagConstraints(0, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    zoomPanel.add(zoomLabel,   new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    zoomPanel.add(zoomSlider, new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
  }

  void phiPanel_mouseClicked(MouseEvent e) {
    if (e.getButton() == MouseEvent.BUTTON1) {
      Set selected = phiPanel.sitesFromPoint(e.getPoint());
      // updateSiteList(selected);
    } else if (e.getButton() == MouseEvent.BUTTON2) {
    }
  }



  /**
   * The X position where the left mouse button was last pressed, for use
   * with rubberbanding.
   */
  private int mousePressedX_ = -1;

  /**
   * The last Y position where the left mouse button was last pressed, for
   * rubber banding purposes.
   */
  private int mousePressedY_ = -1;

  /**
   * To what X the last rubberbanded box was drawn.
   */
  private int boxDrawnX_;

  /**
   * To what Y the last rubberbanded box was drawn.
   */
  private int boxDrawnY_;

  JLabel statusLabel = new JLabel();
  JLabel errorsLabel = new JLabel();
  JSlider zoomSlider = new JSlider();
  JLabel zoomLabel = new JLabel();
  JPanel mapPanel = new JPanel();
  GridBagLayout mapGridBagLayout1 = new GridBagLayout();
  JPanel zoomPanel = new JPanel();
  GridBagLayout gridBagLayout1 = new GridBagLayout();

  /**
   * The text displaying the number of tuples received in the current query.
   */
  JTextField tuplesField = new JTextField();

  /**
   * The text displaying the number of errors received in the current query.
   */
  JTextField errorsField = new JTextField();

  /**
   * True if the current selection is the main selection.  False otherwise.
   */
  private boolean mainSelectionCurrent_;

  void phiPanel_mouseDragged(MouseEvent e) {
    if ((e.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) ==
        MouseEvent.BUTTON1_DOWN_MASK) {
      if (mousePressedX_ != -1) {
        Graphics g = phiPanel.getGraphics();
        g.setXORMode(Color.WHITE);
        if (boxDrawnX_ != -1) {
          g.drawRect(mousePressedX_, mousePressedY_,
                     boxDrawnX_ - mousePressedX_,
                     boxDrawnY_ - mousePressedY_);
        }
        boxDrawnX_ = e.getX();
        boxDrawnY_ = e.getY();
        g.drawRect(mousePressedX_, mousePressedY_,
                   boxDrawnX_ - mousePressedX_,
                   boxDrawnY_ - mousePressedY_);
        phiPanel.repaint();
      }
    } else if ((e.getModifiersEx() & MouseEvent.BUTTON2_DOWN_MASK) ==
               MouseEvent.BUTTON2_DOWN_MASK) {
      Point p = phiPanel.getLocation();
      p.x += e.getX() - mousePressedX_;
      p.y += e.getY() - mousePressedY_;
      phiScroller.getHorizontalScrollBar().setValue(-p.x);
      phiScroller.getVerticalScrollBar().setValue(-p.y);
    }
  }

  void phiPanel_mousePressed(MouseEvent e) {
    // Keep track of where the mouse was pressed, in case we're rubberbanding
    // for a zoom
    mousePressedX_ = e.getX();
    mousePressedY_ = e.getY();
    boxDrawnX_ = -1;
    boxDrawnY_ = -1;
  }

  void phiPanel_mouseReleased(MouseEvent e) {
    // If the first mouse button is released and we're currently
    // rubberbanding, kill the square
    if (e.getButton() == MouseEvent.BUTTON1) {
      if (boxDrawnX_ != -1) {
        Graphics g = phiPanel.getGraphics();
        g.setXORMode(Color.WHITE);
        g.drawRect(mousePressedX_, mousePressedY_,
                   boxDrawnX_ - mousePressedX_,
                   boxDrawnY_ - mousePressedY_);

        // Figure out who should be selected
        Set selected = phiPanel.selectedInRectangle(mousePressedX_,
                                                    mousePressedY_,
                                                    boxDrawnX_ - mousePressedX_,
                                                    boxDrawnY_ - mousePressedY_);
        // updateSiteList(selected);

        phiPanel.repaint();

        // Clean up
        boxDrawnX_ = -1;
        boxDrawnY_ = -1;
      }

      mousePressedX_ = -1;
      mousePressedY_ = -1;
    }
  }

  /**
   * Updates the site list with the current selection
   *
   * @param selected The set of selected sites.
  private void updateSiteList(Set selected) {
    ListSelectionModel model = endPointList.getSelectionModel();
    model.setValueIsAdjusting(true);
    model.clearSelection();
    Iterator iter = selected.iterator();
    while (iter.hasNext()) {
      SITE site = (SITE) iter.next();
      int index = planetLabHelper_.indexFromName(site.getNAME());
      model.addSelectionInterval(index, index);
    }
    model.setValueIsAdjusting(false);
  }
   */

  void zoomSlider_stateChanged(ChangeEvent e) {
    int zoomValue = ((JSlider) e.getSource()).getValue(); // -10 to 10
    double exponent = ((double) zoomValue) / 5;//varies from -2 to 2
    double resizeValue = Math.pow(2.0, exponent);

    // perform the zoom
    phiPanel.zoom(resizeValue);
    phiScroller.setPreferredSize(phiPanel.getPreferredSize());
    phiScroller.validate();
  }


class PhiVizFrame_phiPanel_mouseAdapter extends java.awt.event.MouseAdapter {
  PhiVizFrame adaptee;

  PhiVizFrame_phiPanel_mouseAdapter(PhiVizFrame adaptee) {
    this.adaptee = adaptee;
  }
  public void mouseClicked(MouseEvent e) {
    adaptee.phiPanel_mouseClicked(e);
  }
  public void mousePressed(MouseEvent e) {
    adaptee.phiPanel_mousePressed(e);
  }
  public void mouseReleased(MouseEvent e) {
    adaptee.phiPanel_mouseReleased(e);
  }
}

class PhiVizFrame_phiPanel_mouseMotionAdapter extends java.awt.event.MouseMotionAdapter {
  PhiVizFrame adaptee;

  PhiVizFrame_phiPanel_mouseMotionAdapter(PhiVizFrame adaptee) {
    this.adaptee = adaptee;
  }
  public void mouseDragged(MouseEvent e) {
    adaptee.phiPanel_mouseDragged(e);
  }
}

class PhiVizFrame_zoomSlider_changeAdapter implements javax.swing.event.ChangeListener {
  PhiVizFrame adaptee;

  PhiVizFrame_zoomSlider_changeAdapter(PhiVizFrame adaptee) {
    this.adaptee = adaptee;
  }
  public void stateChanged(ChangeEvent e) {
    adaptee.zoomSlider_stateChanged(e);
  }
}
}
