# Makefile: C++ version of multi-head attention function in transformers 
# Copyright (c) 2024 University of Virginia
# This file is licensed under the MIT License.
# See the LICENSE file in the root of this repository for more details.

LIBPIMEVAL_PATH = ../../../libpimeval
include ../../Makefile.common

# make USE_OPENMP=1
USE_OPENMP = 0
ifeq ($(USE_OPENMP),1)
	CXXFLAGS += -fopenmp
endif

CXXFLAGS += -g
EXEC := mha-emulation.out
SRC := mha-emulation.cpp

debug perf dramsim3_integ: $(EXEC)

$(EXEC): $(SRC) $(DEPS)
	$(CXX) $< $(CXXFLAGS) -o $@

clean:
	rm -rf $(EXEC) *.dSYM
