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
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "encode_kpm_indication.cpp"
#include "build_e2ap_indication.cpp"

using json = nlohmann::json;
using namespace std;

struct KPIEntry {
  std::string timestamp;
  double rx_brate_uplink_Mbps;
  int ul_n_samples;
};

int create_sctp_socket(const std::string& ip, int port) {
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
  if (sock < 0) {
    perror("socket");
    return -1;
  }
  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);
  if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("connect");
    close(sock);
    return -1;
  }
  return sock;
}

void send_kpm_sctp(int sock, const std::vector<uint8_t>& msg) {
  ssize_t sent = sctp_sendmsg(sock, msg.data(), msg.size(), nullptr, 0, 0, 0, 0, 0, 0);
  if (sent < 0) perror("sctp_sendmsg");
}

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

void simulate_kpi_transmission(const std::map<std::string, std::vector<KPIEntry>>& traces, int sctp_sock) {
  const int interval_ms = 250;
  size_t max_entries = 0;
  for (const auto& [slice, vec] : traces)
    max_entries = std::max(max_entries, vec.size());

  for (size_t i = 0; i < max_entries; ++i) {
    for (const auto& [slice, vec] : traces) {
      if (i < vec.size()) {
        const auto& kpi = vec[i];
        std::vector<uint8_t> kpm_encoded, e2ap_encoded;
        if (encode_kpm_indication(slice, kpi.rx_brate_uplink_Mbps, kpi.ul_n_samples, kpm_encoded)) {
          std::cout << "[ENCODED] Slice: " << slice << " | KPM Bytes: " << kpm_encoded.size();
          if (build_e2ap_indication(kpm_encoded, e2ap_encoded)) {
            std::cout << " | E2AP Bytes: " << e2ap_encoded.size() << std::endl;
            send_kpm_sctp(sctp_sock, e2ap_encoded);
          } else {
            std::cerr << "\n[ERROR] Failed to build E2AP Indication message" << std::endl;
          }
        } else {
          std::cerr << "[ERROR] Failed to encode KPM message for slice: " << slice << std::endl;
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
  }
}

int main(int argc, char* argv[]) {
  std::string trace_file = "/opt/e2sim/kpm_e2sm/kpi_traces.json";
  std::string ric_ip = "service-ricplt-e2term-sctp-headless.ricplt.svc";
  int ric_port = 36422;
  if (argc >= 2) ric_ip = argv[1];
  if (argc >= 3) ric_port = std::stoi(argv[2]);
  int sctp_sock = create_sctp_socket(ric_ip, ric_port);
  if (sctp_sock < 0) {
    std::cerr << "[FATAL] Could not connect to RIC at " << ric_ip << ":" << ric_port << std::endl;
    return 1;
  }
  auto traces = load_kpi_data(trace_file);
  simulate_kpi_transmission(traces, sctp_sock);
  close(sctp_sock);
  return 0;
}
