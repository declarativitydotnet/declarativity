package phiviz;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;

public class PlanetLabHelper {

  public static void main(String[] args) throws Exception {
    PlanetLabHelper helper = new PlanetLabHelper();
  }

  final public int NODE_ID    = 0;
  final public int NODE_NAME  = 1;
  final public int NODE_MODEL = 2;
  final public int NODE_VER   = 3;
  final public int NODE_STATE = 4;
  final public int NODE_IP    = 5;
  final public int NODE_MAC   = 6;
  final public int SITE_ID    = 7;
  final public int SITE_NAME  = 8;
  final public int SITE_LONG  = 9;
  final public int SITE_LAT   = 10;
  final public int SITE_URL   = 11;

  /** Defines a list of SITE objects. */
  protected static Hashtable _sites = new Hashtable();


  /**
   * Create a planetlab helper given a sites XML file.
   *
   * @param sitesFile The filename of the XML sites file.
   * @throws IOException
   */
  public PlanetLabHelper() throws IOException
  {
    Runtime runtime = Runtime.getRuntime();
    Process proc;

    try {
      proc = runtime.exec("python sites.py irb_p2");
      proc.waitFor();
    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    } catch (InterruptedException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }



    BufferedReader br = new BufferedReader(new FileReader("sites.out"));
    for (String line = br.readLine(); line != null; line = br.readLine()) {
      String[] node_info = line.split(",,");
      SITE site = new SITE(node_info[SITE_ID], node_info[SITE_NAME],
                           node_info[SITE_LONG], node_info[SITE_LAT],
                           node_info[SITE_URL]);
      if (_sites.containsKey(node_info[SITE_ID])) {
        site = (SITE) _sites.get(node_info[SITE_ID]);
      }
      else {
        _sites.put(node_info[SITE_ID], site);
      }
      HOST host = new HOST(node_info[NODE_ID], node_info[NODE_NAME],
                           node_info[NODE_MODEL], node_info[NODE_VER],
                           node_info[NODE_STATE], node_info[NODE_IP],
                           node_info[NODE_MAC]);
      site.addHOST(host);
    }
    this.buildHostKeyedIndices();
    this.buildSiteByName();
  }

  public Collection sites() {
    return _sites.values();
  }

  /**
   * A map from a host address to the containing PlanetLab site.
   */
  private static HashMap siteFromHostIP_;

  /**
   * A map from a site name to the site itself.
   */
  private static TreeMap siteFromName_;

  /**
   * Maps host names to HOST structures.
   */
  private static TreeMap hostFromName_;

  /**
   * Maps host IP address to HOST structures.
   */
  private static TreeMap hostFromIP_;


  /**
   * Build the indices keyed by host IP and host names.
   */
  private void buildHostKeyedIndices() {
    siteFromHostIP_ = new HashMap();
    hostFromName_ = new TreeMap();
    hostFromIP_ = new TreeMap();
    Iterator iter = _sites.values().iterator();
    while (iter.hasNext()) {
      SITE site = (SITE)iter.next();
      Iterator hostIterator = site.getHOSTList().iterator();
      while (hostIterator.hasNext()) {
        HOST host = (HOST)hostIterator.next();
        siteFromHostIP_.put(host.getIP(), site);
        hostFromName_.put(host.getNAME(), host);
        hostFromIP_.put(host.getIP(), host);
      }
    }
  }

  /**
   * Create the list of site names
   */
  public void buildSiteByName()
  {
    siteFromName_ = new TreeMap();

    // Populate the map
    Iterator iterator = _sites.values().iterator();
    while (iterator.hasNext()) {
      SITE site = (SITE) iterator.next();
      siteFromName_.put(site.getNAME(), new SiteRecord(site));
    }

    // Update the site records with the site indices in the site name array
    siteNames_ = (String[]) siteFromName_.keySet().toArray(new String[0]);
    for (int i = 0;
         i < siteNames_.length;
         i++) {
      SiteRecord record = (SiteRecord) siteFromName_.get(siteNames_[i]);
      record.index = i;
    }
  }

  /**
   * An array of all site names.
   */
  private String[] siteNames_;

  /**
   * An array of pointers to SITE objects that corresponds to the site names
   * array.
   *
   * @see siteNames_
   */
  private SITE[] siteNameBackPointers_;

  /**
   * list
   *
   * @return List
   */
  public Collection list() {
    return _sites.values();
  }

  /**
   * Resolves a host name to its enclosing site, if one exists.
   *
   * @param hostIP The address of the host, in String form.
   * @return A SITE structure of the enclosing site, or null if not found.
   */
  public SITE siteFromHostIP(String hostIP) {
    return (SITE) siteFromHostIP_.get(hostIP);
  }

  /**
   * Return a site given its name, if one exists.
   *
   * @param name The site name.
   * @return A SITE structure, if found.  null otherwise.
   */
  public SITE siteFromName(String name) {
    SiteRecord record = (SiteRecord) siteFromName_.get(name);
    if (record == null) {
      return null;
    } else {
      return record.site;
    }
  }

  /**
    * Return a host given its name, if one exists.
    *
    * @param name The host name.
    * @return A HOST structure, if found.  null otherwise.
    */
   public HOST hostFromName(String name) {
     return (HOST) hostFromName_.get(name);
   }

  /**
   * Fetch the index within the site names array of a given site name.
   *
   * @param siteName The site name.
   * @return The index of the site name in the array. -1 if no such site name
   *   exists.
   */
  public int indexFromName(String siteName) {
    SiteRecord record = (SiteRecord) siteFromName_.get(siteName);
    if (record == null) {
      return -1;
    } else {
      return record.index;
    }
  }

  /**
   * Returns an array of all site names.
   *
   * @return The String array.
   */
  public String[] sitenames() {
    return siteNames_;
  }

  /**
   * <p>Title: PhiViz</p>
   *
   * <p>Description: Phi Visualization</p>
   *
   * <p>Copyright: Copyright (c) 2004</p>
   *
   * <p>Company: Intel Corporation</p>
   * @author Petros Maniatis
   * @version 1.0
   */
  private class SiteRecord {
    private SITE site;
    private int index;
    SiteRecord(SITE site) {
      this.site = site;
    }
  }

}
