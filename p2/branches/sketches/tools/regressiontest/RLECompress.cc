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
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " file block_size." << std::endl;
		return -1;
	}

	std::ifstream fin(argv[1]);
	if (! fin)
	{
		std::cerr << "Cannot open file " << argv[1] << std::endl;
		return -1;
	}

	uint32_t N = atol(argv[2]);

	fin.seekg(0, std::ios::end);
	size_t length = fin.tellg();
	fin.seekg(0, std::ios::beg);

	byte* buffer = 0;
	buffer = new byte[length];

	try
	{
		fin.read(reinterpret_cast<char*>(buffer), length);
		fin.close();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		delete[] buffer;
		return -1;
	}

	byte* data;
	size_t l;

	try
	{
		Tools::compressRLE(N, buffer, length, &data, l);

		std::cerr << "Original size is " << length << " bytes." << std::endl;
		std::cerr << "Compressed size is " << l << " bytes." << std::endl;
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		delete[] buffer;
		delete[] data;
		return -1;
	}

	byte* buffer2;
	size_t length2;

	try
	{
		Tools::uncompressRLE(N, data, l, &buffer2, length2);

		if (length != length2 || memcmp(buffer, buffer2, length) != 0)
		{
			std::cerr << "Error: Uncompression failed!" << length << " " << length2 << std::endl;
			std::cerr << "Please report this problem to " << PACKAGE_BUGREPORT << "." << std::endl;
		}
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		delete[] buffer;
		delete[] data;
		delete[] buffer2;
		return -1;
	}
	
	delete[] buffer;
	delete[] data;
	delete[] buffer2;

	return 0;
}

