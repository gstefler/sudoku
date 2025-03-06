#include "solver.h"
#include <cstring>
#include <sstream>
#include <cmath>

SudokuSolver::SudokuSolver(const char *init) : solutionCount(0), solveTime(0.0)
{
    int length = 0;
    if (!validateInput(init, length))
    {
        throw std::invalid_argument("Invalid Sudoku puzzle input");
    }

    size = static_cast<int>(std::sqrt(length));
    boxSize = static_cast<int>(std::sqrt(size));

    maxSolutions = 10;
    taskDepthThreshold = 4;

    board = std::vector<std::vector<int>>(size, std::vector<int>(size, 0));

    for (int i = 0; i < size * size; i++)
    {
        int x = i % size;
        int y = i / size;
        if (init[i] < '0' || init[i] > '9')
        {
            board[y][x] = 0;
        }
        else
        {
            board[y][x] = init[i] - '0';
        }
    }

    originalBoard = board;
}

bool SudokuSolver::validateInput(const char *init, int &length)
{
    if (!init)
    {
        return false;
    }

    length = std::strlen(init);

    if (!isPerfectSquare(length))
    {
        return false;
    }

    for (int i = 0; i < length; i++)
    {
        if (init[i] != '.' && init[i] != '0' && (init[i] < '1' || init[i] > '9'))
        {
            return false;
        }
    }

    return true;
}

bool SudokuSolver::isPerfectSquare(int num) const
{
    int sqrt_num = static_cast<int>(std::sqrt(num));
    return sqrt_num * sqrt_num == num;
}

void SudokuSolver::solve(int maxSols)
{
    solutions.clear();
    solutionCount.store(0);
    maxSolutions = maxSols;

    auto start = std::chrono::steady_clock::now();

#pragma omp parallel
    {
#pragma omp single nowait
        {
            int row, col;
            // Locate the first empty cell in the board
            if (findBestEmptyCell(board, row, col))
            {
                // For each valid candidate for this cell, spawn a task
                for (int num = 1; num <= size; num++)
                {
                    if (isValid(board, row, col, num))
                    {
                        std::vector<std::vector<int>> boardCopy = board;
                        boardCopy[row][col] = num; // Place the candidate number

#pragma omp task untied firstprivate(boardCopy)
                        {
                            solveSudokuRec(boardCopy, 1);
                        }
                    }
                }
            }
            else
            {
// If the board is already complete, add it as a solution
#pragma omp critical
                {
                    if (solutionCount.load() < maxSolutions)
                    {
                        solutions.push_back(board);
                        solutionCount++;
                    }
                }
            }
        }
#pragma omp taskwait
    }

    auto end = std::chrono::steady_clock::now();
    solveTime = end - start;
}

bool SudokuSolver::isValid(const std::vector<std::vector<int>> &board, int row, int col, int num) const
{
    // Check row
    for (int i = 0; i < size; i++)
    {
        if (board[row][i] == num)
            return false;
    }

    // Check column
    for (int i = 0; i < size; i++)
    {
        if (board[i][col] == num)
            return false;
    }

    // Check box
    int startRow = row - row % boxSize;
    int startCol = col - col % boxSize;
    for (int i = 0; i < boxSize; i++)
    {
        for (int j = 0; j < boxSize; j++)
        {
            if (board[startRow + i][startCol + j] == num)
                return false;
        }
    }

    return true;
}

bool SudokuSolver::findBestEmptyCell(const std::vector<std::vector<int>> &board, int &row, int &col) const
{
    int minOptions = size + 1;
    bool found = false;

    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            if (board[r][c] == 0)
            {
                int options = 0;
                for (int num = 1; num <= size; num++)
                {
                    if (isValid(board, r, c, num))
                    {
                        options++;
                    }
                }

                if (options < minOptions)
                {
                    minOptions = options;
                    row = r;
                    col = c;
                    found = true;
                    if (options == 1)
                        return true;
                }
            }
        }
    }
    return found;
}

void SudokuSolver::solveSudokuRec(std::vector<std::vector<int>> board, int depth)
{
    if ((depth % 5 == 0) && solutionCount.load() >= maxSolutions)
        return;

    int row, col;

    if (!findBestEmptyCell(board, row, col))
    {
#pragma omp critical
        {
            if (solutionCount.load() < maxSolutions)
            {
                solutions.push_back(board);
                solutionCount++;
            }
        }
        return;
    }

    for (int num = 1; num <= size; num++)
    {
        if (isValid(board, row, col, num))
        {
            board[row][col] = num;

            if (depth < taskDepthThreshold)
            {
#pragma omp task firstprivate(board)
                {
                    solveSudokuRec(board, depth + 1);
                }
            }
            else
            {
                solveSudokuRec(board, depth + 1);
            }
            board[row][col] = 0; // Backtrack: reset the cell to empty
        }
    }
}

void SudokuSolver::printSolutions() const
{
    std::cout << "Found " << solutions.size() << " solution(s):\n";
    for (size_t idx = 0; idx < solutions.size(); idx++)
    {
        std::cout << "Solution #" << idx + 1 << ":\n";
        for (const auto &row : solutions[idx])
        {
            for (const auto &val : row)
            {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
        std::cout << "---------------------\n";
    }
}

size_t SudokuSolver::getSolutionCount() const
{
    return solutions.size();
}

const std::vector<std::vector<int>> &SudokuSolver::getSolution(size_t index) const
{
    if (index >= solutions.size())
    {
        throw std::out_of_range("Solution index out of range");
    }
    return solutions[index];
}

const std::vector<std::vector<std::vector<int>>> &SudokuSolver::getAllSolutions() const
{
    return solutions;
}

std::string SudokuSolver::getSolveTime() const
{
    std::ostringstream oss;
    if (solveTime < std::chrono::milliseconds(1))
    {
        oss << std::chrono::duration_cast<std::chrono::microseconds>(solveTime).count() << " qs";
    }
    else if (solveTime < std::chrono::seconds(1))
    {
        oss << std::chrono::duration_cast<std::chrono::milliseconds>(solveTime).count() << " ms";
    }
    else
    {
        oss << std::chrono::duration_cast<std::chrono::seconds>(solveTime).count() << " s";
    }
    return oss.str();
}

void SudokuSolver::printOriginalBoard() const
{
    std::cout << "Original board:\n";
    for (const auto &row : originalBoard)
    {
        for (const auto &val : row)
        {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "---------------------\n";
}
