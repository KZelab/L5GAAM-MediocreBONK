#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Logger.h"

namespace MediocreBONK::Utils
{
    class Profiler
    {
    public:
        struct ProfileResult
        {
            std::string name;
            long long duration; // Microseconds
        };

        static void start(const std::string& name)
        {
            startTimePoints[name] = std::chrono::high_resolution_clock::now();
        }

        static void stop(const std::string& name)
        {
            auto endTimePoint = std::chrono::high_resolution_clock::now();
            auto start = startTimePoints.find(name);
            if (start != startTimePoints.end())
            {
                long long startWait = std::chrono::time_point_cast<std::chrono::microseconds>(start->second).time_since_epoch().count();
                long long endWait = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();
                
                long long duration = endWait - startWait;
                results[name] += duration;
                counts[name]++;
            }
        }

        static void reset()
        {
            results.clear();
            counts.clear();
        }

        static void logResults()
        {
            Logger::info("=== Profiling Results (Avg per frame) ===");
            for (const auto& result : results)
            {
                long long count = counts[result.first];
                if (count > 0)
                {
                    long long avg = result.second / count;
                    Logger::info(result.first + ": " + std::to_string(avg) + "us");
                }
            }
            Logger::info("=========================================");
            reset();
        }

    private:
        static std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> startTimePoints;
        static std::unordered_map<std::string, long long> results;
        static std::unordered_map<std::string, long long> counts;
    };

    // Define static members
    inline std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> Profiler::startTimePoints;
    inline std::unordered_map<std::string, long long> Profiler::results;
    inline std::unordered_map<std::string, long long> Profiler::counts;
}
