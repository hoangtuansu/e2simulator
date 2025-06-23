#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>
#include "E2Sim.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    std::string config_file = "config.json";
    bool verbose = false;

    const struct option long_opts[] = {
        {"help",    no_argument,       nullptr, 'h'},
        {"config",  required_argument, nullptr, 'c'},
        {"verbose", no_argument,       nullptr, 'v'},
        {nullptr,   0,                 nullptr,  0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hc:v", long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                std::cout << "Usage: e2sim [options]\n";
                return 0;
            case 'c':
                config_file = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            default:
                return 1;
        }
    }

    try {
        std::ifstream file(config_file);
        json config = json::parse(file);

        E2Sim sim;
        sim.set_verbose(verbose);
        sim.load_config(config);
        sim.run();

    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
