// Sketches Library
//
// Copyright (C) 2005 Marios Hadjieleftheriou
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

#include <Sketches.h>

int main(int argc, char** argv)
{
	if (argc != 6)
	{
		std::cerr
			<< std::endl
			<< "Usage: CountMinFM " 
			<< "stream_length domain_size skew counters hashes.\n"
			<< "Parameters:\n"
			<< "  stream_length: Total number of insertions.\n"
			<< "  domain_size:   Total number of distinct elements.\n"
			<< "  skew:          The skew parameter of the Zipf distribution.\n"
			<< "  counters:      The number of counters in the filter.\n"
			<< "  hashes:        The number of hashes used per insertion.\n"
			<< std::endl;
		return -1;
	}

	size_t N = atol(argv[1]);
	size_t domainSize = atol(argv[2]);
	double skew  = atof(argv[3]);
	size_t counters = atol(argv[4]);
	size_t hashes = atol(argv[5]);

	Tools::Random r;
	Tools::PRGZipf zipf(0L, domainSize, skew, &r);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	try
	{
		std::cerr << "Testing insertions." << std::endl;

		std::map<std::string, uint64_t> exact;
		std::map<std::string, uint64_t>::iterator itEx;

		for (size_t i = 1; i <= N; i++)
		{
			uint32_t l = zipf.nextLong();
			std::ostringstream ss;
			ss << l << std::flush;

			itEx = exact.find(ss.str());
			if (itEx != exact.end())
				(*itEx).second++;
			else
				exact[ss.str()] = 1;
		}

		Sketches::CountMinFM cmfm(1, exact, counters, hashes, 32, 64, r);

		for (itEx = exact.begin(); itEx != exact.end(); itEx++)
		{
			uint64_t a = cmfm.getFrequency(itEx->first);
			double error = ((a > itEx->second) ? a - itEx->second : itEx->second - a) / itEx->second;
			std::cerr << "Id: " << itEx->first << ", E / A " << itEx->second << " / " << a << ", Error: " << error << std::endl;
		}
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
