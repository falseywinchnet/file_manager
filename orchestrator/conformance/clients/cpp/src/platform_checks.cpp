#include "fileman_orchestrator/client.hpp"

#include <iostream>
#include <string>

int main() {
    namespace orc = fileman::orchestrator;
    orc::SettingsSnapshotInfo settings;
    settings.values.push_back({"navigation.show_hidden", true});
    if (settings.find("navigation.show_hidden") != &settings.values.front().value ||
        settings.find("missing") != nullptr) {
        std::cerr << "settings snapshot lookup failed\n";
        return 1;
    }
    orc::ServicesSnapshotInfo services;
    orc::ServiceInfo service;
    service.id = "engine";
    services.services.push_back(service);
    if (services.find("engine") != &services.services.front() ||
        services.find("missing") != nullptr) {
        std::cerr << "service snapshot lookup failed\n";
        return 1;
    }
#if !defined(__unix__) && !defined(__APPLE__) && !defined(_WIN32)
    try {
        (void)orc::Client::connect_default();
        std::cerr << "unimplemented transport unexpectedly connected\n";
        return 1;
    } catch (const orc::ClientError& error) {
        if (std::string(error.what()).find("requires a Unix-domain socket platform") ==
            std::string::npos) return 1;
    }
    if (orc::BootstrapSnapshot{}.orchestrator_gate_ready()) return 1;
#endif
    return 0;
}
