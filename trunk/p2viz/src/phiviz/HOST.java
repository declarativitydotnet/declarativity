package phiviz;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;

/**
 * The HOST class represents the element "HOST" with the content
 * model defined as follows:
 * <p>
 * &lt;!ELEMENT HOST EMPTY&gt;<br>
 */
public class HOST
{
  public static HOST NONE = new HOST("0x0", "none", "none", "none", "none", "none", "none");

	private SITE   _parent;

	private String  _guid = null;
	private String  _node_id;
	private String _hostname;
	private String _version;

	private String _ip;
	private String _model;
	private String _mac;
	private String _boot_state;

	public HashMap _fingers    = new HashMap();
	public HashSet _successors = new HashSet();
	public boolean _installed = false;

	/**
	 * Creates an empty HOST object.
	 */
	public HOST(String id, String hostname, String model, String version, String boot_state, String ip, String mac)
	{
		super();
		_node_id    = id;
		_hostname   = hostname;
		_model      = model;
		_version    = version;
		_boot_state = boot_state;
		_ip         = ip;
		_mac        = mac;
                System.out.println("New Host " +
                                   hostname);

	}

	public void addFinger(String pos, HOST link) {
		_fingers.put(pos, link);
	}

	public void addSuccessor(HOST s) {
		_successors.add(s);
	}

	public void clearFingers() {
		_fingers.clear();
	}

	public Collection fingers() {
		return _fingers.values();
	}

	public void clearSuccessors() {
		_successors.clear();
	}

	public Collection successors() {
		return _successors;
	}


	public SITE getParent()
	{
		return _parent;
	}

	public void setParent(SITE site)
	{
		_parent = site;
	}

	/**
	 * Gets the value of "IP" attribute.
	 */
	public String getIP()
	{
		return _ip;
	}


	/**
	 * Gets the value of "MODEL" attribute.
	 */
	public String getMODEL()
	{
		return _model;
	}


	/**
	 * Gets the value of "MAC" attribute.
	 */
	public String getMAC()
	{
		return _mac;
	}


	/**
	 * Gets the value of "BOOT_STATE" attribute.
	 */
	public String getBOOTSTATE()
	{
		return _boot_state;
	}

	/**
	 * Gets the value of "NODE_ID" attribute.
	 */
	public String getNODEID()
	{
		return _node_id;
	}

	public void setGUID(String guid) {
		_guid = guid;
	}

	public String getGUID() {
		return _guid;
	}


	/**
	 * Gets the value of "NAME" attribute.
	 */
	public String getNAME()
	{
		return _hostname;
	}


	/**
	 * Gets the value of "VERSION" attribute.
	 */
	public String getVERSION()
	{
		return _version;
	}
}
