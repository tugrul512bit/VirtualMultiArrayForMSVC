#define _AMD64_

#include "GraphicsCardSupplyDepot.h"
#include "VirtualMultiArray.h"
#include "PcieBandwidthBenchmarker.h"
#include "CpuBenchmarker.h"

// testing
#include <random>
#include <iostream>
#include "omp.h"
#include <fstream>
#include <cstring>

constexpr bool TEST_BANDWIDTH = true;
constexpr bool TEST_LATENCY = false;
constexpr bool testType = TEST_BANDWIDTH;

constexpr int TEST_OBJ_SIZE = 1024 * 60;

class Object
{
public:
	Object() :id(-1) {}
	Object(int p) :id(p) {
		constexpr int size = TEST_OBJ_SIZE - sizeof(int);
		data[id % size] = 'A';
	}
	const int getId() const {
		constexpr int size = TEST_OBJ_SIZE - sizeof(int);
		if (data[id % size] == 'A')
			return id;
		else
			return -1;
	}

	Object(const Object& o) = default;
	Object(Object&& o) = default;
	Object& operator = (Object&& o) = default;
	Object& operator = (const Object& o) = default;



private:

	char data[TEST_OBJ_SIZE - sizeof(int)];
	int id;
};

int main(int argc, const char* argv[])
{

	const size_t numThr = 24;
	std::vector<int> numLRU = { 12,12,12,12 }; // 12 OpenCL data channels + 12 LRU caches per physical GPU 
	int totalLRUs = 0;
	for (const auto& e : numLRU)
		totalLRUs += e;
	const long long pageSize = 1; // cache line size (elements)
	const int pagesPerLRU = 50; // cache lines
	int numElementsForAllLRUs = totalLRUs * pagesPerLRU * pageSize;
	const long long n = numElementsForAllLRUs * 100;
	std::cout << "24-thread random-access performance benchmark." << std::endl;
	std::cout << "Element size = " << TEST_OBJ_SIZE << " bytes" << std::endl;
	std::cout << "Array size = " << n * sizeof(Object) << " bytes" << std::endl;
	std::cout << "Cache size = " << numElementsForAllLRUs * sizeof(Object) << " bytes" << std::endl;
	VirtualMultiArray<Object> test(n, GraphicsCardSupplyDepot().requestGpus(), pageSize, pagesPerLRU, numLRU);


	// init
	std::cout << "init..." << std::endl;

	struct Graph2D
	{
		float x;
		float y;
		Graph2D() :x(0), y(0) {}
		Graph2D(float a, float b) :x(a), y(b) {}
	};
	std::vector<Graph2D> log1;
	std::vector<Graph2D> log2;
#pragma omp parallel for num_threads(24)
	for (long long j = 0; j < n; j++)
	{
		test.set(j, Object(j));
	}
	std::cout << "...complete" << std::endl;


	// benchmark
	double limit = (numElementsForAllLRUs / 100 >= 2) ? (numElementsForAllLRUs / 100) : 2;
	for (double size = n - 1; size >= limit; size *= 0.95)
	{

		double hitRatio = (numElementsForAllLRUs / (double)size) * (100.0);
		int numTestsPerThread = 8000 / numThr;

		std::string benchString;
		if (hitRatio < 100.001)
		{
			benchString = std::string("hit-rate=") + std::to_string(hitRatio) + std::string("%");
		}
		else
		{
			benchString = std::string("cache size=") + std::to_string((hitRatio / 100.0)) + std::string("x of data set");
		}

		double seconds = 0;

		{
			CpuBenchmarker bench(numThr * numTestsPerThread * sizeof(Object), benchString);
			bench.addTimeWriteTarget(&seconds);

#pragma omp parallel for num_threads(numThr)
			for (int i = 0; i < numThr; i++)
			{
				double optimizationStopper = 0;
				std::random_device rd;
				std::mt19937 rng(rd());
				std::uniform_real_distribution<float> rnd(0, size);
				{
					for (int k = 0; k < numTestsPerThread; k++)
					{
						int rndIndex = rnd(rng);
						const Object&& obj = test.get(rndIndex);
						if (obj.getId() != rndIndex)
						{
							throw std::invalid_argument("Error: index != data");
						}
					}
				}
			}
		}
		// MB/s per hit ratio
		log1.push_back(Graph2D(hitRatio, (numThr * numTestsPerThread * sizeof(Object) / seconds) / 1000000.0));

		// MB/s per data set size
		log2.push_back(Graph2D(size * sizeof(Object) / 1000000.0, (numThr * numTestsPerThread * sizeof(Object) / seconds) / 1000000.0));
	}

	std::ofstream logFile("logfileHitRatioVsBandwidth.txt", std::ios_base::app | std::ios_base::out);
	for (const auto& point : log1)
	{
		std::string line;
		line += std::to_string(point.x);
		line += std::string(" ");
		line += std::to_string(point.y);
		line += std::string("\r\n");
		logFile << std::string(line);
	}
	std::ofstream logFile2("logfileDataSetSizeVsBandwidth.txt", std::ios_base::app | std::ios_base::out);
	for (const auto& point : log2)
	{
		std::string line;
		line += std::to_string(point.x);
		line += std::string(" ");
		line += std::to_string(point.y);
		line += std::string("\r\n");
		logFile2 << std::string(line);
	}
	return 0;
}