// Tools Library
//
// Copyright (C) 2004  Navel Ltd.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#include <Tools.h>

int main(int argc, char** argv)
{
	Tools::PropertySet p;
	Tools::Variant v;

	v.m_varType = Tools::VT_LONG;
	v.m_val.lVal = -5123456L;
	p.setProperty("Long", v);

	v.m_varType = Tools::VT_BYTE;
	v.m_val.bVal = 'b';
	p.setProperty("Byte", v);

	v.m_varType = Tools::VT_SHORT;
	v.m_val.iVal = -123;
	p.setProperty("Short", v);

	v.m_varType = Tools::VT_FLOAT;
	v.m_val.fltVal = 5.4f;
	p.setProperty("Float", v);

	v.m_varType = Tools::VT_DOUBLE;
	v.m_val.dblVal = 5.4;
	p.setProperty("Double", v);

	v.m_varType = Tools::VT_CHAR;
	v.m_val.cVal = 'a';
	p.setProperty("Char", v);

	v.m_varType = Tools::VT_USHORT;
	v.m_val.uiVal = 123;
	p.setProperty("UShort", v);

	v.m_varType = Tools::VT_ULONG;
	v.m_val.ulVal = 1234556L;
	p.setProperty("ULong", v);

	v.m_varType = Tools::VT_BOOL;
	v.m_val.blVal= true;
	p.setProperty("Bool", v);

	size_t len;
	byte* data;
	p.storeToByteArray(&data, len);

	Tools::PropertySet p2;
	p2.loadFromByteArray(data);
	delete[] data;

	std::cout << p << std::endl;
	std::cout << p2 << std::endl;

	return 0;
}

