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
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " input_file." << std::endl;
		return -1;
	}

	std::ifstream fin(argv[1]);
	if (! fin)
	{
		std::cerr << "Cannot open file " << argv[1] << "." << std::endl;
		return -1;
	}

	Tools::SHA1Hash sha;
	size_t l;
	byte* data;

	sha.hash(fin, &data, l);

	for (size_t i = 0; i < l; i++)
	{
		// FIXME: I cannot figure out the manipulators.
		// They don't appear to work, unless if
		// I call them every single time I print
		// something.
		std::cout
			<< std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(data[i]);
	}

	std::cout << std::endl;

	return 0;
}

