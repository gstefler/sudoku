# 
# Example Makefile to compile and run
# should be modified according to the tools of the paralellization

# Source files 
SRCS = sudoku.cpp solver.cpp
# Generates the object file names from SRCS
OBJS = $(SRCS:.cpp=.o)

CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -fopenmp -O3
LDFLAGS = -fopenmp -O3
LIBS = 

# compile
sudoku: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

# extra dependency 
solver.o: solver.h

# pattern rules 
%.o:	%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@


.PHONY:  easy5
easy5:	sudoku
	./sudoku `head -5 easy.sdk`

.PHONY:  easyAll
easyAll:	sudoku
	./sudoku `cat easy.sdk`

.PHONY:  h5
h5:	sudoku
	./sudoku `head -5 hard.sdk`

.PHONY:  hAll
hAll:	sudoku
	./sudoku `cat hard.sdk`

.PHONY:  hhAll
hhAll:	sudoku
	./sudoku `cat hardest.sdk`

.PHONY:  allProblems
allProblems:	sudoku
	./sudoku `cat all.sdk`

.PHONY:  run1
run1:	sudoku
	OMP_NUM_THREADS=1 srun --ntasks=1 --cpus-per-task=1 -o s1.out ./sudoku -f all.sdk --silent

.PHONY:  run2
run2:	sudoku
	OMP_NUM_THREADS=2 srun --ntasks=1 --cpus-per-task=2 -o s2.out ./sudoku -f all.sdk --silent

.PHONY:  run5
run7:	sudoku
	OMP_NUM_THREADS=5 srun --ntasks=1 --cpus-per-task=5 -o s5.out ./sudoku -f all.sdk --silent

.PHONY:  run8
run8:	sudoku
	OMP_NUM_THREADS=8 srun --ntasks=1 --cpus-per-task=8 -o s8.out ./sudoku -f all.sdk --silent

.PHONY:  run16
run16:	sudoku
	OMP_NUM_THREADS=16 srun --ntasks=1 --cpus-per-task=16 -o s16.out ./sudoku -f all.sdk --silent

.PHONY:  run32
run32:	sudoku
	OMP_NUM_THREADS=32 srun --ntasks=1 --cpus-per-task=32 -o s32.out ./sudoku -f all.sdk --silent

.PHONY:  run64
run64:	sudoku
	OMP_NUM_THREADS=64 srun --ntasks=1 --cpus-per-task=64 -o s64.out ./sudoku -f all.sdk --silent

.PHONY:  run128
run128:	sudoku
	OMP_NUM_THREADS=128 srun --ntasks=1 --cpus-per-task=128 -o s128.out ./sudoku -f all.sdk --silent

.PHONY: clean
clean:
	rm -rf sudoku *.o


