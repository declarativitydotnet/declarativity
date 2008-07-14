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
		std::cerr << "Usage: " << argv[0] << " iterations." << std::endl;
		return -1;
	}

	uint32_t iterations = atol(argv[1]);

	Tools::Random r;
	Tools::UniversalHash h1(r);
	Tools::UniversalHash h2(r);

	for (uint32_t i = 0; i < iterations; i++)
	{
		std::cout
			<< "UH1: "
			<< std::setfill('0') << std::setw(10) << h1.hash(i) << ", "
			<< "UH2: "
			<< std::setfill('0') << std::setw(10) << h2.hash(i)
			<< std::endl;
	}

	return 0;
}

