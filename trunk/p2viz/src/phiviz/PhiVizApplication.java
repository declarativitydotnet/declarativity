package phiviz;

import java.awt.*;
import javax.swing.*;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.image.*;
import javax.imageio.ImageIO;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.math.BigInteger;
import java.net.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.Set;
import java.util.HashSet;

/**
 * <p>
 * Title: PhiViz
 * </p>
 * <p>
 * Description: Phi Visualization
 * </p>
 * <p>
 * Copyright: Copyright (c) 2004
 * </p>
 * <p>
 * Company: Intel Corporation
 * </p>
 *
 * @author Petros Maniatis
 * @version 1.0
 */

public class PhiVizApplication {

  public static Vis p2vis;

  public static PhiVizFrame frame;

  public static PlanetLabHelper sites;
  public static Set TOKEN = new HashSet();

  // Construct the application
  /**
   * PhiVizApplication. The main shell of PhiViz
   *
   * @param mapImage
   *            The image of the map forming the background.
   */
  public PhiVizApplication(BufferedImage mapImage,
                           String sliceName,
                           Vis vis)
      throws IOException {
    p2vis = vis;
    sites = new PlanetLabHelper(sliceName);
    frame = new PhiVizFrame(mapImage, sites);
    create_menus(frame, sites);

    frame.setSize(mapImage.getWidth(), mapImage.getHeight());
    frame.validate();

    // Center the window
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    Dimension frameSize = frame.getSize();
    if (frameSize.height > screenSize.height) {
      frameSize.height = screenSize.height;
    }
    if (frameSize.width > screenSize.width) {
      frameSize.width = screenSize.width;
    }
    frame.setLocation((screenSize.width - frameSize.width) / 2,
                      (screenSize.height - frameSize.height) / 2);
    frame.setVisible(true);
    p2vis.redraw();
  }

  protected void create_menus(final PhiVizFrame frame,
                              final PlanetLabHelper pl) {
    JMenuBar menu_bar = frame.getJMenuBar();

    JMenu menu = menu_bar.getMenu(0);

    JMenuItem item;
    item = new JMenuItem("Show Leaf Sets");
    menu.add(item);
    item.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        for (Iterator i = pl.sites().iterator(); i.hasNext();) {
          SITE s = (SITE) i.next();
          s.showLeafs(true);
        }
        frame.update();
      }
    });

    item = new JMenuItem("Hide Leaf Sets");
    menu.add(item);
    item.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        for (Iterator i = pl.sites().iterator(); i.hasNext();) {
          SITE s = (SITE) i.next();
          s.showLeafs(false);
        }
        frame.update();
      }
    });

    menu.addSeparator();

    item = new JMenuItem("Show Fingers");
    menu.add(item);
    item.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        for (Iterator i = pl.sites().iterator(); i.hasNext();) {
          SITE s = (SITE) i.next();
          s.showFingers(true);
        }
        frame.update();
      }
    });

    item = new JMenuItem("Hide Fingers");
    menu.add(item);
    item.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        for (Iterator i = pl.sites().iterator(); i.hasNext();) {
          SITE s = (SITE) i.next();
          s.showFingers(false);
        }
        frame.update();
      }
    });
  }

	public void eventLoop(ServerSocket net) {
		/*
		Thread painter = new Thread(new Runnable() {
			public void run() {
				while (true) {
					try {
						synchronized (frame) {
							synchronized (p2vis) {
								frame.update();
								p2vis.redraw();
							}
						}
						Thread.sleep(100);
					} catch (Exception e) {
						// TODO Auto-generated catch block
						// e.printStackTrace();
					}
				}
			}
		});
		painter.start();
		*/

		while (true) {
			System.err.println("ACCEPTING CONNECTIONS");
			Socket sock;
			try {
				sock = net.accept();
				ConnectionHandler h = new ConnectionHandler(sock);
				new Thread(h).start();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

		}
	}

	class ConnectionHandler implements Runnable {
          private Socket connection_;

          public ConnectionHandler(Socket sock) {
            connection_ = sock;
          }

          public void run() {
            LineNumberReader reader;
            try {
              reader = new LineNumberReader(new InputStreamReader(connection_
                  .getInputStream()));
            } catch (IOException e1) {
              // TODO Auto-generated catch block
              e1.printStackTrace();
              return;
            }

            while (true) {
              try {
                if (connection_.getInputStream().available() > 0) {
                  String line;
                  while ((line = reader.readLine()) != null) {
                    System.err.println("RECEIVED MESSAGE '" + line + "'");
                    if (line.length() > 0) {
                      String[] tuple = line.split(",");
                      switch (line.charAt(0)) {
                        case 'S':
                          synchronized (frame) {
                            synchronized (p2vis) {
                              handleSuccessor(tuple);
                              frame.update();
                              p2vis.redraw();
                            }
                          }
                          break;
                        case 'T':
                          synchronized (frame) {
                            synchronized (p2vis) {
                              handleToken(tuple);
                              frame.update();
                              p2vis.redraw();
                            }
                          }
                          break;
                        case 'F':
                          synchronized (frame) {
                            synchronized (p2vis) {
                              handleFinger(tuple);
                              frame.update();
                              p2vis.redraw();
                            }
                          }
                          break;
                        case 'I':
                          synchronized (frame) {
                            synchronized (p2vis) {
                              handleInstall(tuple);
                              frame.update();
                              p2vis.redraw();
                            }
                          }
                          break;
                        case 'P':
                          synchronized (frame) {
                            synchronized (p2vis) {
                              handlePath(tuple);
                              frame.update();
                              p2vis.redraw();
                            }
                          }
                          break;
                        default:
                          System.err
                              .println("UNKNOWN TUPLE: " + line);
                      }
                    }
                  }
                  System.err.println("NO MORE INPUT");
                }
              } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
              }
            }
          }

		private void handlePath(String[] tuple) {
			ArrayList<SITE> path = new ArrayList<SITE>();
			for (int i = 1; i < tuple.length; i++) {
				String hostname = tuple[i].split(":")[0];
				HOST host = sites.hostFromName(hostname);
				if (host == null) {
					System.err.println("HANDLE PATH: Unknown hostname -> " + hostname);
					return;
				}
				path.add(host.getParent());
			}
			frame.panel().lookup(path);
		}

		private void handleInstall(String[] tuple) {
                  String hostname = tuple[1].split(":")[0];
                  String guid = tuple[2];
                  HOST host = sites.hostFromName(hostname);
                  if (host != null) {
                    if (host._installed)
                      System.err.println("HOST ALREADY INSTALLED!! " + hostname);
                    host._installed = true;
                  } else {
                    System.err.println("HOST " + hostname
                                       + " DOES NOT EXIST IN VISUALIZER!");
                    return;
                  }
                  p2vis.new_node(guid, host);
		}

                private void handleSuccessor(String[] tuple) {
                  String srcName = tuple[1].split(":")[0];
                  String succName = tuple[2].split(":")[0];

                  HOST src = sites.hostFromName(srcName);
                  HOST succ = sites.hostFromName(succName);
                  if (src != null && succ != null) {
                    if (!src._installed)
                      System.err.println("HOST NOT INSTALLED: " + srcName);
                    if (!succ._installed)
                      System.err.println("HOST NOT INSTALLED: " + succName);
                    src.clearSuccessors();
                    src.addSuccessor(succ);
                  } else
                    System.err.println("HOST IS NULL");
                }

                private void handleToken(String[] tuple) {
                  String pathNodes[] = tuple[2].split("\\+");
                  TOKEN.clear();
                  for (int i = 0; i < pathNodes.length; i++) {
                    String thisNodeSpec = pathNodes[i];
                    String thisNode = thisNodeSpec.split(":")[0];
                    HOST tokenHost =
                        sites.hostFromName(thisNode);
                    TOKEN.add(tokenHost);
                  }
                }

		private void handleFinger(String[] tuple) {
			String pos = tuple[1];
			String srcName = tuple[2].split(":")[0];
			String destName = tuple[3].split(":")[0];

			HOST src = sites.hostFromName(srcName);
			HOST dest = sites.hostFromName(destName);
			if (src != null && dest != null) {
				if (!src._installed)
					System.err.println("HOST NOT INSTALLED: " + srcName);
				if (!dest._installed)
					System.err.println("HOST NOT INSTALLED: " + destName);
				src.addFinger(pos, dest);
			} else
				System.err.println("HOST IS NULL");
		}
	}

        // Main method
        public static void main(String[] args) throws Exception {
          if (args.length != 2) {
            throw new RuntimeException("Expecting <mapFilename> <slicename>");
          }
          String mapFilename = args[0];
          String sliceName = args[1];

          BufferedImage mapImage = ImageIO.read(new FileInputStream(mapFilename));

          try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
          } catch (Exception e) {
            e.printStackTrace();
          }

          Vis vis = new Vis(BigInteger.valueOf(2).pow(160));
          PhiVizApplication app = new PhiVizApplication(mapImage,
                                                        sliceName,
                                                        vis);
          app.eventLoop(new ServerSocket(10001));
        }
}
