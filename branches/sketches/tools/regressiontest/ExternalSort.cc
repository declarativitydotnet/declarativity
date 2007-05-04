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

class MyObject : public Tools::IObject, public Tools::IComparable, public Tools::ISerializable
{
public:
	MyObject(double xlow, double ylow, double xhigh, double yhigh)
		: m_xlow(xlow), m_xhigh(xhigh), m_ylow(ylow), m_yhigh(yhigh) {}

	virtual ~MyObject() {}

	virtual IObject* clone() throw (Tools::NotSupportedException)
	{
		return new MyObject(m_xlow, m_ylow, m_xhigh, m_yhigh);
	}

	virtual bool operator<(const Tools::IComparable& c) const
	{
		const MyObject* po = static_cast<const MyObject*>(&c);
		if (m_xlow < po->m_xlow) return true;
		return false;
	}

	virtual bool operator>(const Tools::IComparable& c) const
	{
		const MyObject* po = static_cast<const MyObject*>(&c);
		if (m_xlow > po->m_xlow) return true;
		return false;
	}

	virtual bool operator==(const Tools::IComparable& c) const
	{
		const MyObject* po = static_cast<const MyObject*>(&c);
		if (m_xlow == po->m_xlow) return true;
		return false;
	}

	virtual size_t getByteArraySize()
	{
		return 4 * sizeof(double);
	}

	virtual void loadFromByteArray(const byte* ptr)
	{
		memcpy(&m_xlow, ptr, sizeof(double));
		ptr += sizeof(double);
		memcpy(&m_ylow, ptr, sizeof(double));
		ptr += sizeof(double);
		memcpy(&m_xhigh, ptr, sizeof(double));
		ptr += sizeof(double);
		memcpy(&m_yhigh, ptr, sizeof(double));
		//data += sizeof(double);
	}

	virtual void storeToByteArray(byte** data, size_t& len)
	{
		len = getByteArraySize();
		*data = new byte[len];
		byte* ptr = *data;
		memcpy(ptr, &m_xlow, sizeof(double));
		ptr += sizeof(double);
		memcpy(ptr, &m_ylow, sizeof(double));
		ptr += sizeof(double);
		memcpy(ptr, &m_xhigh, sizeof(double));
		ptr += sizeof(double);
		memcpy(ptr, &m_yhigh, sizeof(double));
		//ptr += sizeof(double);
	}

	double m_xlow, m_xhigh, m_ylow, m_yhigh;
};

class MyObjectStream : public Tools::IObjectStream
{
public:
	MyObjectStream(std::string inputFile) : m_bEOF(false)
	{
		m_fin.open(inputFile.c_str());
		if (! m_fin)
		{
			throw Tools::IllegalArgumentException("Input file not found.");
		}
	}

	virtual Tools::IObject* getNext()
	{
		if (m_bEOF) return 0;

		double d[4];
		m_fin >> d[0] >> d[1] >> d[2] >> d[3];

		if (! m_fin.good())
		{
			m_bEOF = true;
			return 0;
		}

		return new MyObject(d[0], d[1], d[2], d[3]);
	}

	virtual bool hasNext() throw (Tools::NotSupportedException)
	{
		throw Tools::NotSupportedException("Operation not supported.");
	}

	virtual size_t size() throw (Tools::NotSupportedException)
	{
		throw Tools::NotSupportedException("Operation not supported.");
	}

	virtual void rewind()
	{
		m_fin.seekg(0, std::ios::beg);
		if (!m_fin.good()) m_bEOF = true;
		else m_bEOF = false;
	}

	std::ifstream m_fin;
	bool m_bEOF;
};

int main(int argc, char** argv)
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: " << argv[0] << " input_file buffer_size." << std::endl;
			return -1;
		}

		size_t bufferSize = atoll(argv[2]);

		MyObjectStream sin(argv[1]);

		Tools::SmartPointer<Tools::IObjectStream> sout(Tools::externalSort(sin, bufferSize));

		std::cout << std::fixed;

		MyObject* o;
		while ((o = dynamic_cast<MyObject*>(sout->getNext())) != 0)
		{
			std::cout
				<< o->m_xlow << " " << o->m_ylow << " "
				<< o->m_xhigh << " " << o->m_yhigh << std::endl;
			delete o;
		}
	}
	catch (Tools::Exception& e)
	{
		std::cerr << "******ERROR******" << std::endl;
		std::string s = e.what();
		std::cerr << s << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cerr << "******ERROR******" << std::endl;
		std::cerr << "other exception" << std::endl;
		return -1;
	}

	return 0;
}
