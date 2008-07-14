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
			<< "Usage: LossyCounting " 
			<< "stream_length domain_size skew epsilon theta.\n"
			<< "Parameters:\n"
			<< "  stream_length: Total number of insertions.\n"
			<< "  domain_size:   Total number of distinct elements.\n"
			<< "  skew:          The skew parameter of the Zipf distribution.\n"
			<< "  epsilon:       The accuracy level of the sketch.\n"
			<< "                 Specifies how large each window will be.\n"
			<< "  theta:         The frequency threshold.\n"
			<< "                 An item is considered frequent if f >= theta * L.\n"
			<< "                 This parameter is used for querying the sketch.\n"
			<< "                 It does not affect the construction of the sketch.\n"
			<< std::endl;
		return -1;
	}

	size_t N = atol(argv[1]);
	size_t domainSize = atol(argv[2]);
	double skew  = atof(argv[3]);
	double epsilon = atof(argv[4]);
	double theta = atof(argv[5]);

	Sketches::LossyCounting lc(epsilon);
	std::map<std::string, uint64_t> exact;
	std::map<std::string, uint64_t>::iterator itEx;

	Tools::Random r;
	Tools::PRGZipf zipf(0L, domainSize, skew, &r);

	double totalError = 0.0;
	size_t totalItems = 0;
	size_t maxSketchSize = 0;

	std::cerr << std::setprecision(4) << std::fixed;
	std::cout << std::setprecision(4) << std::fixed;

	try
	{
		for (size_t i = 1; i <= N; i++)
		{
			uint32_t l = zipf.nextLong();
			std::ostringstream ss;
			ss << l << std::flush;

			lc.insert(ss.str());

			itEx = exact.find(ss.str());
			if (itEx != exact.end()) (*itEx).second++;
			else exact[ss.str()] = 1;

			if (i % 100 == 0)
			{
				maxSketchSize = std::max(maxSketchSize, lc.getNumberOfEntries());

				if (lc.getInputLength() != i)
					throw Tools::IllegalStateException(
						"Error: getInputLength mismatch."
					);

				std::map<
					std::string,
					std::pair<uint64_t, uint64_t> > m;

				m = lc.getFrequent(static_cast<uint64_t>(theta * i));

				std::map<
					std::string,
					std::pair<uint64_t, uint64_t>
				>::iterator itM;

				for (itM = m.begin(); itM != m.end(); itM++)
				{
					itEx = exact.find((*itM).first);
					assert(itEx != exact.end());

					double re =
						std::abs(
							static_cast<double>(
								(*itEx).second - (*itM).second.first
							)
						) / static_cast<double>((*itEx).second);

					totalError += re;
					totalItems++;

					std::cout
						<< "Id: " << (*itEx).first << ", "
						<< "Exact: " << (*itEx).second << ", "
						<< "Estimate: " << (*itM).second.first << ", "
						<< "Delta: " << (*itM).second.second << ", "
						<< "Relative error: " << re
						<< std::endl;
				}

				std::cerr
					<< "Insertions: " << i << ", "
					<< "Sketch size: " << lc.getNumberOfEntries() << ", "
					<< "Frequent elements: " << m.size() << ", "
					<< "Relative error: " << totalError / totalItems
					<< std::endl;
			}
		}

		std::cerr
			<< "Average Relative Error: "
			<< totalError / totalItems << std::endl
			<< "Maximum sketch size (in number of entries): "
			<< 2 * maxSketchSize << std::endl
				// every entry is two uint64_t, count and delta.
			<< "Exact solution size: " << exact.size() << std::endl;
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

