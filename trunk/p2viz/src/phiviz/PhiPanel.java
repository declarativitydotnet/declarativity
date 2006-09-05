package phiviz;

/**
 * <p>Title: PhiViz</p>
 * <p>Description: Phi Visualization</p>
 * <p>Copyright: Copyright (c) 2004</p>
 * <p>Company: Intel Corporation</p>
 * @author Petros Maniatis
 * @version 1.0
 */

import java.util.*;
import java.util.List;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.*;
import javax.swing.*;

public class PhiPanel
    extends JPanel
{
  /**
   * The image filling up the panel.  It is an appropriately zoomed version of
   * the map, with the sites drawn on top after the zoom.
   */
  private BufferedImage image_;

  private static final Color UNINSTALLED_COLOR = Color.WHITE;
  private static final Color INSTALLED_COLOR = Color.BLUE;
  private static final Color SUCC_COLOR = Color.GREEN;
  private static final Color FINGER_COLOR = Color.RED;
  private static final Color PATH_COLOR = Color.CYAN;



  /**
   * The background image (i.e., the map)
   */
  private BufferedImage background_;

  /**
   * The dimensions of the image, as zoomed in or out
   */
  private Dimension imageDimension_;

  /**
   * The PlanetLab sites structure
   */
  private PlanetLabHelper sites_;
  
  private ArrayList<SITE> lookupPath_ = null;

  /**
   * The current zoom factor
   */
  private double zoomFactor_;

  /**
   * The side of the square drawn around sites in pixels.
   */
  private static final int SIDE = PhiVizFrame.SIDE;

  /**
   * Create the CanPanel
   *
   * @param mapImage The background image (i.e., the map).
   * @param sites The sites structure for all of the planetlab nodes, as read
   *   from the XML file.
   */
  public PhiPanel(BufferedImage mapImage, PlanetLabHelper sites)
  {
    background_ = mapImage;
    sites_ = sites;
    imageDimension_ = new Dimension(background_.getWidth(),
                                    background_.getHeight());
    zoom(1.0);
  }
  
  public void lookup(ArrayList<SITE> path) {
	  lookupPath_ = path;
  }

  /**
   * drawSites. Draw the sites from the current site structure onto the
   * appropriately resized panel.
   */
  private void drawSites() {
    Graphics graphics = image_.getGraphics();
    Collection siteList = sites_.list();
    Iterator iterator = siteList.iterator();
    int height = (int) imageDimension_.getHeight();
    int width = (int) imageDimension_.getWidth();
    graphics.setColor(Color.WHITE);
    graphics.drawLine(width / 2, 0, width / 2, height);
    graphics.drawLine(0, height / 2, width, height / 2);

    while (iterator.hasNext()) {
      SITE site = (SITE) iterator.next();
   	  graphics.setColor(UNINSTALLED_COLOR);
      for (int i = 0; i < site.getHOSTCount(); i++) {
    	  HOST host = site.getHOST(i);
    	  if (host._installed) {
    		  graphics.setColor(INSTALLED_COLOR);
    		  break;
    	  }
      }
      double lat = site.getLATITUDE();
      double lon = site.getLONGITUDE();
      double y = height / 2.0 - lat / 90.0 * height / 2;
      double x = width / 2.0 + lon / 180.0 * width / 2;
      graphics.fillRect((int) x - SIDE / 2, (int) y - SIDE / 2, SIDE, SIDE);
    }
  }
  
  private void drawLinks() {
    Collection siteList = sites_.list();
    Iterator iterator = siteList.iterator();

    while (iterator.hasNext()) {
      SITE site = (SITE) iterator.next();
      for (int i = 0; i < site.getHOSTCount(); i++) {
    	  HOST host = site.getHOST(i);
    	  if (site.showLeafs()) {
    	  	Iterator siter = host.successors().iterator();
    	  	while (siter.hasNext()) {
    		  HOST neighbor = (HOST) siter.next();
    		  drawLink(host.getParent(), neighbor.getParent(), SUCC_COLOR);
    	  	}
    	  }
    	  if (site.showFingers()) {
    		Iterator fiter = host.fingers().iterator();
    	  	while (fiter.hasNext()) {
    		  HOST neighbor = (HOST) fiter.next();
    		  drawLink(host.getParent(), neighbor.getParent(), FINGER_COLOR);
    	  	}
    	  }
      }
    }
  }
  
  private void drawPath() {
	  if (lookupPath_ != null) { 
		  for (int i = 0; i < lookupPath_.size()-1; i++) { 
			  SITE src = lookupPath_.get(i);
			  SITE dest = lookupPath_.get(i+1); 
    		drawLink(src, dest, PATH_COLOR);
		  }
	  }
  }
  

  /**
   * Standard component painter.  It draws the foreground image onto the panel.
   *
   * @param g The graphics context of the display.
   */

  /**
   * Standard component painter.  It draws the foreground image onto the panel.
   *
   * @param g The graphics context of the display.
   */
  public void paintComponent(Graphics g)
  {
    super.paintComponent(g); //paint background
    g.drawImage(image_, 0, 0,
                this);
  }

  /**
   * Returns a set of sites selected by (containing) a given point.
   *
   * @param pPoint The point in question.
   * @return A set of SITE objects.
   */
  public Set sitesFromPoint(Point pPoint) {
    // Go through all the sites and figure out within whose vicinity the
    // mouse is
    Collection siteList = sites_.list();
    Iterator iterator = siteList.iterator();
    HashSet selected = new HashSet();
    while (iterator.hasNext()) {
      SITE site = (SITE) iterator.next();
      double lat = site.getLATITUDE();
      double lon = site.getLONGITUDE();
      Point sitePoint =
          geoToXY(lat,lon);

      double mouseX = pPoint.getX();
      double mouseY = pPoint.getY();

      if ( (mouseX >= sitePoint.getX() - SIDE / 2) &&
          (mouseX < sitePoint.getX() + SIDE / 2) &&
          (mouseY >= sitePoint.getY() - SIDE / 2) &&
          (mouseY < sitePoint.getY() + SIDE / 2)) {
       selected.add(site);
      }
    }

    return selected;
  }

  /**
   * Returns a set of entities contained within a rectangle. The entities are
   * sites, if the panel is in victims mode, or ASes if the panel is in
   * attackers mode. Only entities whose icon box is entirely contained within
   * the rectangle are selected.
   *
   * @param x The top left corner's x.
   * @param y The top left corner's y.
   * @param width The width of the box.
   * @param height The height of the box.
   * @return A set of SITE objects if in victims mode or AutonomousSystem
   *   objects if in attackers mode.
   */
  public Set selectedInRectangle(int x,
                                 int y,
                                 int width,
                                 int height) {
    HashSet selected = new HashSet();

      // Go through all the sites and figure out whose selector fits entirely
      // within the box
      Collection siteList = sites_.list();
      Iterator iterator = siteList.iterator();
      while (iterator.hasNext()) {
        SITE site = (SITE) iterator.next();
        double lat = site.getLATITUDE();
        double lon = site.getLONGITUDE();
        Point sitePoint =
            geoToXY(lat, lon);

        if ((x <= sitePoint.getX() - SIDE / 2) &&
            (x + width > sitePoint.getX() + SIDE / 2) &&
            (y <= sitePoint.getY() - SIDE / 2) &&
            (y + height > sitePoint.getY() + SIDE / 2)) {
         selected.add(site);
        }
      }

    return selected;
  }

  /**
   * Point variable used to return geoToXYPoint results.
   */
  private Point geoToXYPoint_ = new Point();

  /**
   * Return the X,Y coordinates in the image coordinate system of a given
   * latitude and longitude pair.
   *
   * @param lat Longitude
   * @param lon Latitude
   * @return The resulting point.  The structure returned is reset with every
   *   invocation of the method, so it should be used immediately.
   */
  private Point geoToXY(double lat,
                       double lon) {
    double y = imageDimension_.getHeight() * (1 - lat / 90.0) / 2;
    double x = imageDimension_.getWidth() * (1 + lon / 180.0) / 2;
    geoToXYPoint_.setLocation(x, y);
    return geoToXYPoint_;
  }

  /**
   * Change the size of the panel, by supplying an absolute zoom factor. It
   * keeps the aspect ratio constant.
   *
   * @param zoomFactor The zoom factor, as a proportion of the true size of the
   *   panel.
   */
  public void zoom(double zoomFactor) {
    int width = (int) (zoomFactor * background_.getWidth());
    int height = (int) (zoomFactor * background_.getHeight());
    zoomFactor_ = zoomFactor;
    image_ =
        new BufferedImage(width, height,
                          BufferedImage.TYPE_INT_RGB);
    image_.createGraphics().drawImage(background_, 0, 0,
                                      width, height, this);
    imageDimension_.setSize(width, height);
    setPreferredSize(imageDimension_);
    setMaximumSize(imageDimension_);

    drawSites();
    drawLinks();
    drawPath();

    invalidate();
    repaint();
  }

  /**
   * Draws an attack path given the scalar weight of the path (between 0 and 1).
   *
   * @param attackerAS The source AS
   * @param victimSite The target SITE.
   * @param factor The scalar factor (0-1)
   */
  private void drawLink(SITE source, SITE dest, double factor, Color c) {
    Graphics graphics = image_.getGraphics();
 	graphics.setColor(c);
    int height = (int) imageDimension_.getHeight();
    int width = (int) imageDimension_.getWidth();


    double srcLat = source.getLATITUDE();
    double srcLon = source.getLONGITUDE();
    double srcY = height / 2.0 - srcLat / 90.0 * height / 2;
    double srcX = width / 2.0 + srcLon / 180.0 * width / 2;

    double destLat = dest.getLATITUDE();
    double destLon = dest.getLONGITUDE();
    double destY = height / 2.0 - destLat / 90.0 * height / 2;
    double destX = width / 2.0 + destLon / 180.0 * width / 2;

    // Draw the edge
    int intFactor = (int) (factor * 255.0);
    Color edgeColor = new Color(intFactor, 100, 255 - intFactor);
    graphics.setColor(edgeColor);
    drawLine(graphics, c==PATH_COLOR, srcX, srcY, destX, destY);
  }


  /**
   * Draw an attack path from the attacker to the victim.
   *
   * @param attackerAS The AS of the attacker
   * @param victimSite The target site
   */
  private void drawLink(SITE source, SITE dest, Color c) {
    Graphics graphics = image_.getGraphics();
 	graphics.setColor(c);
    int height = (int) imageDimension_.getHeight();
    int width = (int) imageDimension_.getWidth();


    double srcLat = source.getLATITUDE();
    double srcLon = source.getLONGITUDE();
    double srcY = height / 2.0 - srcLat / 90.0 * height / 2;
    double srcX = width / 2.0 + srcLon / 180.0 * width / 2;

    double destLat = dest.getLATITUDE();
    double destLon = dest.getLONGITUDE();
    double destY = height / 2.0 - destLat / 90.0 * height / 2;
    double destX = width / 2.0 + destLon / 180.0 * width / 2;

    // Draw the edge
    drawLine(graphics, c==PATH_COLOR, srcX, srcY, destX, destY);
  }

  /**
   * Draws a parabolic path from the attacker to the victim.
   *
   * @param graphics Graphics
   * @param attackerX double
   * @param attackerY double
   * @param victimX double
   * @param victimY double
   */
  private void drawLine(Graphics graphics, boolean straight,
                        double sourceX,
                        double sourceY,
                        double destX,
                        double destY) {
    if (straight) {
    	graphics.drawLine((int)sourceX,(int)sourceY,(int)destX,(int)destY);
    }
    else {
    	double angle = 0;
    	if (sourceX == destX) {
    		// On the same longitude. Their angle is +-90%
    		if (destY > sourceY) {
    		  angle = Math.PI / 2.0;
     	 	} else if (destY < sourceY) {
    		  angle = Math.PI / 2.0;
     	 	}
    	} else {
    		double yDistance = destY - sourceY;
    		double xDistance = destX - sourceX;
    		angle = Math.atan((yDistance) / (xDistance));
    	}
    	double distance = Point2D.distance(sourceX, sourceY, destX, destY);
    	Point2D controlPoint = new Point2D.Double(-distance / 3.0, 0);
    
    	AffineTransform transform = new AffineTransform();
    	transform.translate((sourceX + destX) / 2,
    	                    (sourceY + destY) / 2);
    	transform.rotate(angle + Math.PI / 2.0);
    	transform.transform(controlPoint, controlPoint);
    	QuadCurve2D quadratic = new QuadCurve2D.Double(sourceX, sourceY,
    											controlPoint.getX(),
                                                controlPoint.getY(),
                                                destX, destY);
    	((Graphics2D) graphics).draw(quadratic);
    }
  }

  /**
   * Gets the image width.
   *
   * @return The image width.
   */
  public double imageWidth() {
    return imageDimension_.getWidth();
  }

  /**
   * Repaint the panel with a fresh set of data.
   */
  public void update() {
    zoom(zoomFactor_);
  }
}
