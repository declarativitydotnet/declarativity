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
	if (argc != 8)
	{
		std::cerr
			<< std::endl
			<< "Usage: RangeEfficientF0 " 
			<< "stream_length domain_size skew sample_size number_of_samples interval_size loops.\n"
			<< "Parameters:\n"
			<< "  stream_length:       Total number of insertions.\n"
			<< "  domain_size:         Total number of distinct elements.\n"
			<< "  skew:                The skew parameter of the Zipf distribution.\n"
			<< "  sample_size:         The number entries in the sample.\n"
			<< "  number_of_samples:   The number of samples.\n"
			<< "  interval_size:       The interval size.\n"
			<< "  loops:               The number of individual experiments to run.\n"
			<< std::endl;
		return -1;
	}

	size_t N = atol(argv[1]);
	uint64_t domainSize = atoll(argv[2]);
	double skew  = atof(argv[3]);
	size_t sampleSize = atol(argv[4]);
	size_t numberOfSamples = atol(argv[5]);
	size_t intervalSize = atol(argv[6]);
	size_t loops = atol(argv[7]);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	double totalErrorU = 0.0;
	double minU = std::numeric_limits<double>::max();
	double maxU = 0.0;
	size_t total = 0;
	uint32_t seed = time(0);

	try
	{
		std::cerr << "Testing insertions." << std::endl;

		//uint64_t M = Sketches::getPrime(10 * domainSize, 20 * domainSize);

		for (size_t cLoop = 0; cLoop < loops; cLoop++)
		{
			Tools::Random r(seed);
			Tools::PRGZipf zipf(1ul, domainSize, skew, &r);

			Sketches::RangeEfficientF0 re(sampleSize, numberOfSamples);
			std::set<uint32_t> exact;

			for (size_t i = 1; i <= N; i++)
			{
				uint32_t low = zipf.nextLong();
				uint32_t high = low + r.nextUniformLong(1, intervalSize);

				re.insert(low, high);

				for (uint32_t l = low; l <= high; l++)
					exact.insert(l);
			}

			double errorU = std::abs(static_cast<double>(exact.size()) - static_cast<double>(re.getCount())) / static_cast<double>(exact.size());

			minU = std::min(minU, errorU);
			maxU = std::max(maxU, errorU);
			totalErrorU += errorU;
			total++;

			std::cerr
				<< "Loop: " << cLoop << ", "
				<< "Exact distinct: " << exact.size() << ", "
				<< "RE distinct: " << re.getCount() << ", "
				<< "Relative error: " << errorU
				<< std::endl;
		}

		std::cerr
			<< "Average relative error: "
			<< totalErrorU / total << std::endl
			<< "Minimum error: " << minU << std::endl
			<< "Maximum error: " << maxU << std::endl;
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

