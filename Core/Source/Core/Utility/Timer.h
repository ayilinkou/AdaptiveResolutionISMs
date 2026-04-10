#pragma once

#include <chrono>
#include <string>

class Timer
{
public:
	Timer(const std::string& name)
	{
		start = std::chrono::high_resolution_clock::now();
		this->name = name;
	}

	~Timer()
	{
		EndTimer();
	}

	void EndTimer()
	{
		if (!bFinished)
		{
			bFinished = true;
			end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> duration = end - start;

			printf("%s took %.1f milliseconds.\n", name.c_str(), duration.count() * 1000.f);
		}
	}

private:
	std::string name;
	bool bFinished = false;

	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point end;
};