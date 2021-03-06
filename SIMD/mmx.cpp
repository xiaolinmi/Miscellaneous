// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>

#include <mmintrin.h>

#include <stdlib.h>
#include <assert.h>

#include <fstream>

///////////////////////////

void calculateUsingCpp(char* data, unsigned size)
{
	assert(size % 8 == 0);

	unsigned step = 10;
	for (unsigned i = 0; i < size; ++i)
	{
		*data++ += step;
	}
}

void calculateUsingIntrinsics(char* data, unsigned size)
{
	assert(size % 8 == 0);

	__m64 step = _mm_set_pi8(10, 10, 10, 10, 10, 10, 10, 10);
	__m64* dst = reinterpret_cast<__m64*>(data);
	for (unsigned i = 0; i < size; i += 8)
	{
		auto sum = _mm_adds_pi8(step, *dst);
		*dst++ = sum;
	}

	_mm_empty();
}

void calculateUsingAsm(char* data, unsigned size)
{
	assert(size % 8 == 0);

	unsigned loopCount = size / 8;
	__int64 value = 0x0a0a0a0a0a0a0a0a;
	__asm 
	{
			push eax
			push ecx

			mov eax, data
			mov ecx, loopCount
			movq mm1, value

		startLoop:
			movq mm0, [eax]
			paddb  mm0, mm1
			movq [eax], mm0

			add eax, 8
			dec ecx
			jnz startLoop

			emms
			pop ecx
			pop eax
	}
}

//////////////////////////

LARGE_INTEGER g_counterBegin;
LARGE_INTEGER g_counterEnd;

void beginBenchmark()
{
	if (QueryPerformanceCounter(&g_counterBegin) == 0) {
		fprintf(stderr, "benchmark error:QueryPerformanceCounter");
		exit(-3);
	}
}

void endBenchmark()
{
	if (QueryPerformanceCounter(&g_counterEnd) == 0) {
		fprintf(stderr, "benchmark error:QueryPerformanceCounter");
		exit(-4);
	}
}

void printBenchmarkTime()
{
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency) == 0) {
		fprintf(stderr, "benchmark error:QueryPerformanceFrequency");
		exit(-5);
	}

	fprintf(stdout, "Benchmark time: %f s\n", (g_counterEnd.QuadPart - g_counterBegin.QuadPart) / (float)frequency.QuadPart);
}

/////////////////////////

void saveToFile(const char* filename, char* data, unsigned size)
{
	std::ofstream outfile(filename);
	outfile.write(data, size);
}

/////////////////////////

int main()
{
	constexpr unsigned dataSizeInBytes = 10000000;

	auto data = static_cast<char*>(calloc(dataSizeInBytes, 1));
	if (data == 0) {
		fprintf(stderr, "allocate 10000000-byte memory failed!");
		return -1;
	}

	beginBenchmark();
	calculateUsingCpp(data, dataSizeInBytes);
	endBenchmark();

	saveToFile("calCpp.txt", data, dataSizeInBytes);
	printBenchmarkTime();

	///////

	beginBenchmark();
	calculateUsingIntrinsics(data, dataSizeInBytes);
	endBenchmark();

	saveToFile("calIns.txt", data, dataSizeInBytes);
	printBenchmarkTime();

	///////

	beginBenchmark();
	calculateUsingAsm(data, dataSizeInBytes);
	endBenchmark();

	saveToFile("calAsm.txt", data, dataSizeInBytes);
	printBenchmarkTime();

    return 0;
}

