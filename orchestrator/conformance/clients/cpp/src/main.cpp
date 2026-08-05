#include "fileman_orchestrator/client.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: orchestrator-cpp-client RUNTIME_DIR probe|shutdown\n";
        return 2;
    }
    try {
        auto client = fileman::orchestrator::Client::connect(argv[1]);
        const std::string command = argv[2];
        if (command == "probe") {
            const auto snapshot = client.bootstrap();
            std::cout << "instance=" << snapshot.session.instance_id
                      << " generation=" << snapshot.session.lifecycle_generation
                      << " component=" << snapshot.version.component
                      << " build=" << snapshot.version.build_version
                      << " release=" << snapshot.release.state
                      << " ready=" << (snapshot.release.ready ? "true" : "false")
                      << " lifecycle=" << snapshot.status.lifecycle_state
                      << " capabilities=" << snapshot.availability.size() << '\n';
        } else if (command == "shutdown") {
            client.shutdown();
            std::cout << "shutdown=success\n";
        } else {
            std::cerr << "unknown command: " << command << '\n';
            return 2;
        }
    } catch (const std::exception& error) {
        std::cerr << "orchestrator-cpp-client: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
