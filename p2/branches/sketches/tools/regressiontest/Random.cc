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
	if (argc != 4)
	{
		std::cerr
			<< "Usage: " << argv[0]
			<< " iterations domain_size prg_type (mersenne | drand48)."
			<< std::endl;
		return -1;
	}

	size_t iterations = atoll(argv[1]);
	uint64_t domainSize = atoll(argv[2]);
	Tools::RandomGeneratorType type = Tools::RGT_MERSENNE;
	if (strcmp(argv[3], "drand48") == 0)
		type = Tools::RGT_DRAND48;

	Tools::Random r(time(0), type);

	std::set<long> s;
	for (uint32_t i = 0; i < iterations; i++)
	{
		uint64_t l = r.nextUniformUnsignedLongLong(0, domainSize);
		s.insert(l);
		//std::cerr << l << std::endl;
	}

	std::cerr
		<< std::endl << s.size()
		<< " distinct random numbers produced"
		<< std::endl;

	return 0;
}

