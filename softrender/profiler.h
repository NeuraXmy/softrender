#ifndef PROFILER_H
#define PROFILER_H

#include "utils.h"
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std::string_literals;

class Profiler
{
public:
	
	Profiler(std::string_view name)
	{
		_current_start_time = get_time();
		_name = name;
	}

	~Profiler()
	{
		stop();
	}

	void stop()
	{
		if (!_stoped)
		{
			auto delta = get_time() - _current_start_time;
			switch (_mode)
			{
			case Profiler::Mode::OVERRIDE:
				_times[_name] = { delta, 1 };
				break;
			case Profiler::Mode::AVG:
				_times[_name].first += delta;
				_times[_name].second += 1;
				if (_name == "run fs")
				std::cout << std::fixed << std::setprecision(7)<< delta << std::endl;
				break;
			case Profiler::Mode::SUM:
				_times[_name].first += delta;
				_times[_name].second = 1;
				break;
			}
		}
		_stoped = true;
	}
	

	static double get(std::string_view name)
	{
		auto& p = _times.find(name.data())->second;
		return p.first / p.second;
	}

	static std::string str(bool sorted_by_length = false)
	{
		std::ostringstream out;
		out << std::fixed << std::setprecision(7);
		std::vector<std::pair<std::string, double>> times;
		size_t max_len = 0;
		for (auto& [name, p] : _times)
			times.emplace_back(name, get(name)), max_len = std::max(max_len, name.size());
		if(sorted_by_length)
			std::sort(times.begin(), times.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
		for (auto [name, time] : times)
			out << std::setw(max_len) << name << ": " << time << "\n";
		return out.str();
	}

	enum class Mode
	{
		OVERRIDE,
		AVG,
		SUM
	};

	static void set_mode(Mode m)
	{
		_mode = m;
		clear();
	}

	static void clear()
	{
		_times.clear();
	}

private:

	bool _stoped = false;

	std::string _name;
	
	double _current_start_time;

	inline static std::unordered_map<std::string, std::pair<double, int>> _times;

	inline static Mode _mode = Mode::OVERRIDE;
		
};

#define PROFILE_SCOPE(name) Profiler _profiler(name);

#endif