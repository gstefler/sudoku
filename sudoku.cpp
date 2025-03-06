#include "solver.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <getopt.h>
#include <atomic>
#include <chrono>
#include <thread>

std::vector<std::string> readPuzzlesFromFile(const std::string &filename)
{
    std::vector<std::string> puzzles;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line))
    {
        puzzles.push_back(line);
    }
    return puzzles;
}

int main(int argc, char *argv[])
{
    const char *defaultPuzzle = "....14....3....2...7..........9...3.6.1.............8.2.....1.4....5.6.....7.8...";
    std::vector<std::string> puzzles;
    bool showResult = false;
    bool showOriginal = false;
    bool silent = false;
    bool progressEnabled = false; // new flag for progress bar
    std::string filename;

    struct option long_options[] = {
        {"file", required_argument, 0, 'f'},
        {"result", no_argument, 0, 'r'},
        {"original", no_argument, 0, 'o'},
        {"silent", no_argument, 0, 's'},
        {"progress", no_argument, 0, 'p'}, // new option
        {0, 0, 0, 0}};

    int opt;
    int option_index = 0;
    // add 'p' to getopt string
    while ((opt = getopt_long(argc, argv, "f:rosp", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'f':
            filename = optarg;
            break;
        case 'r':
            showResult = true;
            break;
        case 'o':
            showOriginal = true;
            break;
        case 's':
            silent = true;
            break;
        case 'p': // enable progress bar
            progressEnabled = true;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " [-f filename] [--result] [--original] [--silent] [--progress]" << std::endl;
            return 1;
        }
    }

    if (!filename.empty())
    {
        puzzles = readPuzzlesFromFile(filename);
    }
    else
    {
        puzzles.push_back(defaultPuzzle);
        for (int i = optind; i < argc; i++)
        {
            puzzles.push_back(argv[i]);
        }
    }

    std::atomic<size_t> progressCounter(0);
    const size_t totalPuzzles = puzzles.size();
    const bool useProgressBar = progressEnabled && (totalPuzzles > 80);

    auto startTime = std::chrono::steady_clock::now();
    std::atomic<bool> done(false);

    std::thread progressThread;
    if (useProgressBar)
    {
        progressThread = std::thread([&]()
                                     {
            while (!done.load())
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                size_t solved = progressCounter.load();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - startTime).count();
                double rate = elapsed > 0 ? static_cast<double>(solved) / elapsed : 0;
                double remainingTime = rate > 0 ? (totalPuzzles - solved) / rate : 0;
                std::cout << "\rProgress: " << solved << " / " << totalPuzzles 
                          << " | " << rate << " sudokus/sec"
                          << " | ETA: " << remainingTime << " sec" << std::flush;
            } });
    }

// #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < totalPuzzles; i++)
    {
        try
        {
            SudokuSolver solver(puzzles[i].c_str());
            solver.solve();

            if (!silent)
            {
#pragma omp critical
                {
                    std::cout << "\n"
                              << i << ". solved in " << solver.getSolveTime();
                    if (showOriginal)
                    {
                        solver.printOriginalBoard();
                    }
                    if (showResult)
                    {
                        solver.printSolutions();
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
#pragma omp critical
            {
                std::cerr << "\nError at " << i << ".: " << e.what() << std::endl;
            }
        }
        ++progressCounter;
    }

    done.store(true);
    if (useProgressBar)
    {
        if (progressThread.joinable())
        {
            progressThread.join();
        }
        std::cout << std::endl;
    }

    // Print summary
    auto endTime = std::chrono::steady_clock::now();

    enum class TimeFormat
    {
        Seconds,
        Milliseconds,
        Microseconds
    };
    
    // check the elapsed time if it is less than 1 second then print in milliseconds if it is less than 1 millisecond then print in microseconds
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    auto format = TimeFormat::Seconds;
    if (elapsed < 1)
    {
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        format = TimeFormat::Milliseconds;
        if (elapsed < 1)
        {
            format = TimeFormat::Microseconds;
            elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        }
    }

    // print rate in current format
    double rate = elapsed > 0 ? static_cast<double>(totalPuzzles) / elapsed : 0;

    std::cout << "\nSolved " << totalPuzzles << " puzzles in " << elapsed;
    switch (format)
    {
    case TimeFormat::Seconds:
        std::cout << " seconds";
        break;
    case TimeFormat::Milliseconds:
        std::cout << " milliseconds";
        rate *= 1000; // convert rate to puzzles per second
        break;
    case TimeFormat::Microseconds:
        std::cout << " microseconds";
        rate *= 1000000; // convert rate to puzzles per second
        break;
    }

    // print rate in puzzles/{current format}
    std::cout << " | " << rate << " puzzles/sec" << std::endl;

    return 0;
}