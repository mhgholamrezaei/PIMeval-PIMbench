# Makefile: C++ version of hdc
# Copyright (c) 2024 University of Virginia
# This file is licensed under the MIT License.
# See the LICENSE file in the root of this repository for more details.

PROJ_ROOT = ../../..
include ${PROJ_ROOT}/Makefile.common

# make USE_OPENMP=1
USE_OPENMP ?= 0
ifeq ($(USE_OPENMP),1)
	CXXFLAGS += -fopenmp
endif

EXEC := hdc.out
SRC := hdc.cpp

debug perf dramsim3_integ: $(EXEC)

$(EXEC): $(SRC) $(DEPS)
	$(CXX) $< $(CXXFLAGS) $(LDFLAGS) -o $@

clean:
	rm -rf $(EXEC) *.dSYM
