#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include <cmath>
#include <string>
#include <stdexcept>
#include <omp.h>
#include <chrono>

class SudokuSolver
{
public:
    SudokuSolver(const char *init);

    void solve(int maxSolutions = 10);

    void printSolutions() const;

    size_t getSolutionCount() const;

    const std::vector<std::vector<int>> &getSolution(size_t index) const;

    const std::vector<std::vector<std::vector<int>>> &getAllSolutions() const;

    std::string getSolveTime() const;

    void printOriginalBoard() const;

private:
    int size;
    int boxSize;
    int maxSolutions;
    int taskDepthThreshold;

    std::vector<std::vector<int>> board;
    std::vector<std::vector<int>> originalBoard;
    std::vector<std::vector<std::vector<int>>> solutions;
    std::atomic<int> solutionCount;
    std::chrono::duration<double> solveTime;

    bool isValid(const std::vector<std::vector<int>> &board, int row, int col, int num) const;
    bool findBestEmptyCell(const std::vector<std::vector<int>> &board, int &row, int &col) const;
    void solveSudokuRec(std::vector<std::vector<int>> board, int depth = 0);
    bool isPerfectSquare(int num) const;
    bool validateInput(const char *init, int &length);
};