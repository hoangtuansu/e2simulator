/*****************************************************************************
#                                                                            *
# Copyright 2020 AT&T Intellectual Property                                  *
#                                                                            *
# Licensed under the Apache License, Version 2.0 (the "License");            *
# you may not use this file except in compliance with the License.           *
# You may obtain a copy of the License at                                    *
#                                                                            *
#      http://www.apache.org/licenses/LICENSE-2.0                            *
#                                                                            *
# Unless required by applicable law or agreed to in writing, software        *
# distributed under the License is distributed on an "AS IS" BASIS,          *
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
# See the License for the specific language governing permissions and        *
# limitations under the License.                                             *
#                                                                            *
******************************************************************************/
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

struct KPIEntry {
  std::string timestamp;
  double rx_brate_uplink_Mbps;
  int ul_n_samples;
};

std::map<std::string, std::vector<KPIEntry>> load_kpi_data(const std::string& filepath) {
  std::ifstream ifs(filepath);
  json kpi_json = json::parse(ifs);

  std::map<std::string, std::vector<KPIEntry>> traces;

  for (const auto& [slice_type, entries] : kpi_json.items()) {
    for (const auto& entry : entries) {
      KPIEntry kpi {
        entry["timestamp"],
        entry["rx_brate_uplink_Mbps"],
        entry["ul_n_samples"]
      };
      traces[slice_type].push_back(kpi);
    }
  }
  return traces;
}

void simulate_kpi_transmission(const std::map<std::string, std::vector<KPIEntry>>& traces) {
  const int interval_ms = 250;

  size_t max_entries = 0;
  for (const auto& [slice, vec] : traces)
    max_entries = std::max(max_entries, vec.size());

  for (size_t i = 0; i < max_entries; ++i) {
    for (const auto& [slice, vec] : traces) {
      if (i < vec.size()) {
        const auto& kpi = vec[i];
        std::cout << "[Slice: " << slice << "] "
                  << "Timestamp: " << kpi.timestamp
                  << ", Uplink Rate: " << kpi.rx_brate_uplink_Mbps << " Mbps"
                  << ", UL Samples: " << kpi.ul_n_samples << std::endl;

        // Ici, tu peux encoder ces valeurs dans un message E2SM-KPM
        // et appeler l'API d'envoi du simulateur e2sim
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
  }
}

int main() {
  std::string trace_file = "/opt/e2sim/kpm_e2sm/kpi_traces.json";
  auto traces = load_kpi_data(trace_file);
  simulate_kpi_transmission(traces);
  return 0;
}
