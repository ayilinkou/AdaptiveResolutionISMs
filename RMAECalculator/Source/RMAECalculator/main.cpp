#include <iostream>
#include <stdexcept>

#include "RMAECalculator.h"

int main(int argc, char* argv[])
{
	// ensure enough arguments can contain two file paths to compare
	if (argc < 3)
		return -1;
	
	const char* truth = argv[1];
	const char* test = argv[2];

	double RMAE = -1.0;

	std::cout << "\nGround truth image: " << truth << std::endl;
	std::cout << "Test image: " << test << std::endl;

	try
	{
		RMAE = RMAECalculator::CalculateFromFiles(truth, test);
	}
	catch (const std::invalid_argument& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return -2;
	}

	std::cout << "\nRMAE: " << RMAE << std::endl;

	return 0;
}