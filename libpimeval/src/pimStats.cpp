// File: pimStats.cpp
// PIMeval Simulator - Stats
// Copyright (c) 2024 University of Virginia
// This file is licensed under the MIT License.
// See the LICENSE file in the root of this repository for more details.

#include "pimStats.h"
#include "pimSim.h"
#include "pimUtils.h"
#include <chrono>            // for chrono
#include <cstdint>           // for uint64_t
#include <cstdio>            // for printf
#include <iostream>          // for cout
#include <iomanip>           // for setw, fixed, setprecision
#include <string>
#include <tuple>
#include <fstream>
#include <sstream>

//! @brief  Show PIM stats
void
pimStatsMgr::showStats() const
{
  std::printf("----------------------------------------\n");
  #if defined(DEBUG)
  showApiStats();
  #endif
  showDeviceParams();
  showCopyStats();
  showCmdStats();
  std::printf("----------------------------------------\n");
}

//! @brief  Show API stats
void
pimStatsMgr::showApiStats() const
{
  std::printf("Simulator API Stats:\n");
  std::printf(" %30s : %10s %14s\n", "PIM-API", "CNT", "Elapsed(ms)");
  int totCalls = 0;
  int totCallsDevice = 0;
  int totCallsAlloc = 0;
  int totCallsCopy = 0;
  int totCallsCompute = 0;
  double msTotalElapsed = 0.0;
  double msTotalElapsedDevice = 0.0;
  double msTotalElapsedAlloc = 0.0;
  double msTotalElapsedCopy = 0.0;
  double msTotalElapsedCompute = 0.0;
  for (const auto& it : m_msElapsed) {
    std::printf(" %30s : %10d %14f\n", it.first.c_str(), it.second.first, it.second.second);
    totCalls += it.second.first;
    msTotalElapsed += it.second.second;
    if (it.first.find("createDevice") == 0) {
      totCallsDevice += it.second.first;
      msTotalElapsedDevice += it.second.second;
    } else if (it.first.find("pimAlloc") == 0 || it.first.find("pimFree") == 0) {
      totCallsAlloc += it.second.first;
      msTotalElapsedAlloc += it.second.second;
    } else if (it.first.find("pimCopy") == 0) {
      totCallsCopy += it.second.first;
      msTotalElapsedCopy += it.second.second;
    } else {
      totCallsCompute += it.second.first;
      msTotalElapsedCompute += it.second.second;
    }
  }
  std::printf(" %30s : %10d %14f\n", "TOTAL ---------", totCalls, msTotalElapsed);
  std::printf(" %30s : %10d %14f\n", "TOTAL (Device )", totCallsDevice, msTotalElapsedDevice);
  std::printf(" %30s : %10d %14f\n", "TOTAL ( Alloc )", totCallsAlloc, msTotalElapsedAlloc);
  std::printf(" %30s : %10d %14f\n", "TOTAL (  Copy )", totCallsCopy, msTotalElapsedCopy);
  std::printf(" %30s : %10d %14f\n", "TOTAL (Compute)", totCallsCompute, msTotalElapsedCompute);
}

//! @brief  Show PIM device params
void
pimStatsMgr::showDeviceParams() const
{
  const pimParamsDram& paramsDram = pimSim::get()->getParamsDram();
  std::printf("PIM Params:\n");
  std::printf(" %30s : %s\n", "PIM Device Type Enum",
              pimUtils::pimDeviceEnumToStr(pimSim::get()->getDeviceType()).c_str());
  std::printf(" %30s : %s\n", "PIM Simulation Target",
              pimUtils::pimDeviceEnumToStr(pimSim::get()->getSimTarget()).c_str());
  std::printf(" %30s : %u, %u, %u, %u, %u\n", "Rank, Bank, Subarray, Row, Col",
              pimSim::get()->getNumRanks(),
              pimSim::get()->getNumBankPerRank(),
              pimSim::get()->getNumSubarrayPerBank(),
              pimSim::get()->getNumRowPerSubarray(),
              pimSim::get()->getNumColPerSubarray());
  std::printf(" %30s : %u\n", "Number of PIM Cores", pimSim::get()->getNumCores());
  std::printf(" %30s : %u\n", "Number of Rows per Core", pimSim::get()->getNumRows());
  std::printf(" %30s : %u\n", "Number of Cols per Core", pimSim::get()->getNumCols());
  std::printf(" %30s : %f GB/s\n", "Typical Rank BW", paramsDram.getTypicalRankBW());
  std::printf(" %30s : %f\n", "Row Read (ns)", paramsDram.getNsRowRead());
  std::printf(" %30s : %f\n", "Row Write (ns)", paramsDram.getNsRowWrite());
  std::printf(" %30s : %f\n", "tCCD (ns)", paramsDram.getNsTCCD_S());
  #if defined(DEBUG)
  std::printf(" %30s : %f\n", "AAP (ns)", paramsDram.getNsAAP());
  #endif
}


// Helper function to escape strings for JSON
std::string escapeString(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\t': oss << "\\t"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

// Function to serialize the data to JSON
std::string serializeToJson(const std::map<std::string, std::tuple<int, pimeval::perfEnergy, uint64_t>>& data) {
    std::ostringstream oss;
    oss << "{\n";
    bool first = true;
    for (const auto& [cmdName, tupleData] : data) {
        if (!first) oss << ",\n";
        first = false;

        const auto& [count, perfEnergy, numElements] = tupleData;

        oss << "  \"" << escapeString(cmdName) << "\": {\n"
            << "    \"count\": " << count << ",\n"
            << "    \"numElements\": " << numElements << "\n"
            << "  }";
    }
    oss << "\n}";
    return oss.str();
}

// Function to write JSON to file
void 
pimStatsMgr::writeCmdStatsToJson(const std::string& jsonFileName) {
    std::string jsonString = serializeToJson(m_cmdPerf);
    std::ofstream outFile(jsonFileName);
    if (!outFile.is_open()) {
        throw std::ios_base::failure("Failed to open file: " + jsonFileName);
    }
    outFile << jsonString;
    outFile.close();
}

//! @brief  Show data copy stats
void
pimStatsMgr::showCopyStats() const
{
  std::printf("Data Copy Stats:\n");
  uint64_t bytesCopiedMainToDevice = m_bitsCopiedMainToDevice / 8;
  uint64_t bytesCopiedDeviceToMain = m_bitsCopiedDeviceToMain / 8;
  uint64_t bytesCopiedDeviceToDevice = m_bitsCopiedDeviceToDevice / 8;
  uint64_t totalBytes = bytesCopiedMainToDevice + bytesCopiedDeviceToMain;
  double totalMsRuntime = m_elapsedTimeCopiedMainToDevice + m_elapsedTimeCopiedDeviceToMain + m_elapsedTimeCopiedDeviceToDevice;
  double totalMjEnergy = m_mJCopiedMainToDevice + m_mJCopiedDeviceToMain + m_mJCopiedDeviceToDevice;
  std::cout << std::setw(45) << "Host to Device" << " : " << bytesCopiedMainToDevice << " bytes" << std::endl;
  std::cout << std::setw(45) << "Device to Host" << " : " << bytesCopiedDeviceToMain << " bytes" << std::endl;
  std::cout << std::setw(45) << "Device to Device" << " : " << bytesCopiedDeviceToDevice << " bytes" << std::endl;
  std::cout << std::setw(45) << "TOTAL ---------" << " : " << totalBytes << " bytes "
            << std::setw(14) << std::fixed << std::setprecision(6) << totalMsRuntime << " ms Estimated Runtime "
            << std::setw(14) << std::fixed << std::setprecision(6) << totalMjEnergy << " mj Estimated Energy"
            << std::endl;
}

//! @brief  Show PIM cmd and perf stats
void
pimStatsMgr::showCmdStats() const
{
  std::printf("PIM Command Stats:\n");
  std::printf(" %44s : %10s %14s %20s %14s\n", "PIM-CMD", "CNT", "#Elements", "EstimatedRuntime(ms)", "EstimatedEnergyConsumption(mJ)");
  int totalCmd = 0;
  double totalMsRuntime = 0.0;
  double totalMjEnergy = 0.0;
  for (const auto& it : m_cmdPerf) {
    std::printf(" %44s : %10d %9ld %14f %14f\n", it.first.c_str(), std::get<0>(it.second), std::get<2>(it.second), std::get<1>(it.second).m_msRuntime, std::get<1>(it.second).m_mjEnergy);
    totalCmd += std::get<0>(it.second);
    totalMsRuntime += std::get<1>(it.second).m_msRuntime;
    totalMjEnergy += std::get<1>(it.second).m_mjEnergy;
  }
  std::printf(" %44s : %10d %14f %14f\n", "TOTAL ---------", totalCmd, totalMsRuntime, totalMjEnergy);

  // analyze micro-ops
  int numR = 0;
  int numW = 0;
  int numL = 0;
  int numActivate = 0;
  int numPrecharge = 0;
  for (const auto& it : m_cmdPerf) {
    if (it.first == "row_r") {
      numR += std::get<0>(it.second);
      numActivate += std::get<0>(it.second);
      numPrecharge += std::get<0>(it.second);
    } else if (it.first == "row_w") {
      numW += std::get<0>(it.second);
      numActivate += std::get<0>(it.second);
      numPrecharge += std::get<0>(it.second);
    } else if (it.first.find("rreg.") == 0) {
      numL += std::get<0>(it.second);
    }
  }
  if (numR > 0 || numW > 0 || numL > 0) {
    std::printf(" %30s : %d, %d, %d\n", "Num Read, Write, Logic", numR, numW, numL);
    std::printf(" %30s : %d, %d\n", "Num Activate, Precharge", numActivate, numPrecharge);
  }
}

//! @brief  Reset PIM stats
void
pimStatsMgr::resetStats()
{
  m_cmdPerf.clear();
  m_msElapsed.clear();
  m_bitsCopiedMainToDevice = 0;
  m_bitsCopiedDeviceToMain = 0;
  m_bitsCopiedDeviceToDevice = 0;
}

//! @brief pimPerfMon ctor
pimPerfMon::pimPerfMon(const std::string& tag)
{
  m_startTime = std::chrono::high_resolution_clock::now();
  m_tag = tag;
}

//! @brief pimPerfMon dtor
pimPerfMon::~pimPerfMon()
{
  auto now = std::chrono::high_resolution_clock::now();
  double elapsed = std::chrono::duration<double, std::milli>(now - m_startTime).count();
  pimSim::get()->getStatsMgr()->recordMsElapsed(m_tag, elapsed);
}

