package phiviz;

import java.util.*;

public class SITE
{
	public boolean _show_leaf = true;
	public boolean _show_fingers = true;
	
	public String _site_id;
	public String _name;
	public String _url;

	public double _longitude;
	public double _latitude;

	protected ArrayList _objHOST = new ArrayList();

	/**
	 * Creates an empty SITE object
	 */
	public SITE(String site_id, String name, String longitude, String latitude, String url)
	{
		super();
		_site_id   = site_id;
		_name      = name;
		_url       = url;
		_longitude = Double.parseDouble(longitude);
		_latitude  = Double.parseDouble(latitude);
	}
	
	public void showLeafs(boolean v) {
		_show_leaf = v;
	}
	
	public boolean showLeafs() {
		return _show_leaf;
	}
	
	public void showFingers(boolean v) {
		_show_fingers = v;
	}
	
	public boolean showFingers() {
		return _show_fingers;
	}

	/**
	 * Gets the value of "URL" attribute.
	 */
	public String getURL()
	{
		return _url;
	}

	/**
	 * Gets the value of "SITE_ID" attribute.
	 */
	public String getSITEID()
	{
		return _site_id;
	}

	/**
	 * Gets the value of "LONGITUDE" attribute.
	 */
	public double getLONGITUDE()
	{
		return _longitude;
	}

	/**
	 * Gets the value of "LATITUDE" attribute.
	 */
	public double getLATITUDE()
	{
		return _latitude;
	}

	/**
	 * Gets the value of "NAME" attribute.
	 */
	public String getNAME()
	{
		return _name;
	}

	/**
	 * Returns an array of HOST objects. The length of the returned
	 * array is zero if the list of HOST object is empty.
	 */
	public HOST[] getHOST()
	{
		return (HOST[])_objHOST.toArray(new HOST[0]);
	}

	/**
	 * Replaces all existing HOST objects with a new array of
	 * HOST objects.
	 * @param objArray	an array of HOST objects.
	 */
	public void setHOST(HOST[] objArray)
	{
		if( objArray == null || objArray.length == 0 )
			this._objHOST.clear();
		else
		{
			this._objHOST = new ArrayList(Arrays.asList(objArray));
			for( int i=0; i<objArray.length; i++ )
			{
				if( objArray[i] != null )
					objArray[i].setParent(this);
			}
		}
	}

	/**
	 * Gets the HOST object at the specified index.
	 * @param index	index of the returned object.
	 * @throws IndexOutOfBoundsException	if index is out of range.
	 */
	public HOST getHOST(int index)
	{
		return (HOST)_objHOST.get(index);
	}

	/**
	 * Replaces an existing HOST object at the specified index with
	 * a new HOST object.
	 * @param index	index of replaced object.
	 * @throws IndexOutOfBoundsException	if index is out of range.
	 */
	public void setHOST(int index, HOST obj)
	{
		if( obj == null )
			removeHOST(index);
		else
		{
			_objHOST.set(index, obj);
			obj.setParent(this);
		}
	}

	/**
	 * Returns the number of HOST objects in the list.
	 */
	public int getHOSTCount()
	{
		return _objHOST.size();
	}

	/**
	 * Returns <code>true</code> if there is no HOST object in the list; otherwise,
	 * the method returns <code>false</code>.
	 */
	public boolean isNoHOST()
	{
		return _objHOST.size() == 0;
	}

	/**
	 * Returns a read-only list of HOST objects.
	 */
	public List getHOSTList()
	{
		return Collections.unmodifiableList(_objHOST);
	}

	/**
	 * Adds a new HOST object at the end of the list.
	 * @return <code>true</code> if the new object is added to the list; otherwise,
	 * the method returns <code>false</code>.
	 */
	public boolean addHOST(HOST obj)
	{
		if( obj==null )
			return false;

		obj.setParent(this);
		return _objHOST.add(obj);
	}

	/**
	 * Adds a list of new HOST objects at the end of the list.
	 * @return <code>true</code> if the list was changed; otherwise, the method
	 * returns <code>false</code>.
	 */
	public boolean addHOST(Collection coHOST)
	{
		if( coHOST==null )
			return false;

		java.util.Iterator it = coHOST.iterator();
		while( it.hasNext() )
		{
			Object obj = it.next();
			if( obj != null && obj instanceof HOST )
				((HOST)obj).setParent(this);
		}
		return _objHOST.addAll(coHOST);
	}

	/**
	 * Removes an existing HOST object at the specified index.
	 * @return	The removed object.
	 */
	public HOST removeHOST(int index)
	{
		return (HOST)_objHOST.remove(index);
	}

	/**
	 * Removes the specified HOST object.
	 * @return <code>true</code> if this list contains the object; otherwise,
	 * the method returns <code>false</code>.
	 */
	public boolean removeHOST(HOST obj)
	{
		return _objHOST.remove(obj);
	}

	/**
	 * Clears all HOST objects from the list.
	 */
	public void clearHOSTList()
	{
		_objHOST.clear();
	}
}
