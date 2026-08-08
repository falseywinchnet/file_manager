#include "fileman_orchestrator/client.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3 && argc != 5) {
		std::cerr << "usage: orchestrator-cpp-client [RUNTIME_DIR] probe|shutdown|search ROOT_ID TEXT\n";
        return 2;
    }
    try {
        auto client = argc >= 3 ? fileman::orchestrator::Client::connect(argv[1])
                                : fileman::orchestrator::Client::connect_default();
        const std::string command = argc >= 3 ? argv[2] : argv[1];
        if (command == "probe") {
            const auto snapshot = client.bootstrap();
            std::cout << "instance=" << snapshot.session.instance_id
                      << " generation=" << snapshot.session.lifecycle_generation
                      << " component=" << snapshot.version.component
                      << " build=" << snapshot.version.build_version
                      << " release=" << snapshot.release.state
                      << " ready=" << (snapshot.release.ready ? "true" : "false")
                      << " digest=" << snapshot.release.provenance.digest
                      << " lifecycle=" << snapshot.status.lifecycle_state
                      << " contracts=" << snapshot.contracts.size()
                      << " capabilities=" << snapshot.availability.size()
                      << " route=" << snapshot.routing.normal_integration_route
                      << " fallback=" << snapshot.routing.direct_engine_fallback.state
                      << " shutdown="
                      << (snapshot.service_controls.shutdown_eligible ? "eligible" : "ineligible")
                      << " restart=" << snapshot.service_controls.restart_strategy
                      << " orchestrator-gate="
                      << (snapshot.orchestrator_gate_ready() ? "ready" : "blocked")
                      << " gui-forms-gate=" << snapshot.frontend_opening.gui_forms_gate.state
                      << " architect-gate="
                      << snapshot.frontend_opening.architect_direction_gate.state
                      << " opening-blockers="
                      << snapshot.frontend_opening.orchestrator_gate.blockers.size() << '\n';
        } else if (command == "search" && argc == 5) {
            const auto page = client.search(argv[3], argv[4]);
            std::cout << "terminal=" << page.terminal
                      << " source=" << page.source
                      << " complete=" << (page.complete ? "true" : "false")
                      << " results=" << page.names.size();
            if (!page.names.empty()) std::cout << " first=" << page.names.front();
            std::cout << '\n';
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
