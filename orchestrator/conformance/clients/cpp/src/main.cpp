#include "fileman_orchestrator/client.hpp"

#include <iostream>
#include <optional>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: orchestrator-cpp-client [RUNTIME_DIR] "
                     "probe|shutdown|search ROOT_ID TEXT [MAX_RESULTS "
                     "[CURSOR_SOURCE CURSOR]]|criteria ROOT_ID FIELD VALUE "
                     "[MAX_RESULTS]\n";
        return 2;
    }
    try {
        const std::string first = argv[1];
        const bool default_runtime = first == "probe" || first == "shutdown" ||
            first == "search" || first == "criteria";
        const int command_index = default_runtime ? 1 : 2;
        if (command_index >= argc) {
            std::cerr << "missing command\n";
            return 2;
        }
        auto client = default_runtime
            ? fileman::orchestrator::Client::connect_default()
            : fileman::orchestrator::Client::connect(argv[1]);
        const std::string command = argv[command_index];
        const int argument_index = command_index + 1;
        if (command == "probe") {
            if (argument_index != argc) {
                std::cerr << "probe accepts no arguments\n";
                return 2;
            }
            const auto snapshot = client.bootstrap();
            const auto settings_schema = client.settings_schema();
            const auto settings = client.settings_snapshot();
            const auto services = client.services_snapshot();
            const auto* engine = services.find("engine");
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
                      << snapshot.frontend_opening.orchestrator_gate.blockers.size()
                      << " settings-fields=" << settings_schema.fields.size()
                      << " settings-revision=" << settings.revision
                      << " services=" << services.services.size()
                      << " engine=" << (engine ? engine->state : "missing")
                      << " engine-currentness="
                      << (engine && engine->currentness
                              ? *engine->currentness
                              : "unavailable")
                      << '\n';
        } else if (command == "search") {
            const int remaining = argc - argument_index;
            if (remaining != 2 && remaining != 3 && remaining != 5) {
                std::cerr << "search requires ROOT_ID TEXT [MAX_RESULTS "
                             "[CURSOR_SOURCE CURSOR]]\n";
                return 2;
            }
            auto maximum = 128U;
            if (remaining >= 3) {
                const auto parsed = std::stoul(argv[argument_index + 2]);
                if (parsed == 0UL || parsed > 1'000UL) {
                    std::cerr << "MAX_RESULTS must be in the closed range 1..1000\n";
                    return 2;
                }
                maximum = static_cast<std::uint32_t>(parsed);
            }
            std::optional<fileman::orchestrator::SearchCursorInfo> cursor;
            if (remaining == 5) {
                cursor = fileman::orchestrator::SearchCursorInfo{
                    argv[argument_index + 3], argv[argument_index + 4]};
            }
            const auto page = client.search_subtree(
                argv[argument_index], std::nullopt,
                argv[argument_index + 1], maximum, cursor);
            std::cout << "terminal=" << page.terminal
                      << " source=" << page.source
                      << " complete=" << (page.complete ? "true" : "false")
                      << " results=" << page.names.size();
            if (!page.names.empty()) std::cout << " first=" << page.names.front();
            if (page.cursor) {
                std::cout << " cursor_source=" << page.cursor->source
                          << " cursor=" << page.cursor->value;
            }
            std::cout << '\n';
        } else if (command == "criteria") {
            const int remaining = argc - argument_index;
            if (remaining != 3 && remaining != 4) {
                std::cerr << "criteria requires ROOT_ID FIELD VALUE [MAX_RESULTS]\n";
                return 2;
            }
            auto maximum = 128U;
            if (remaining == 4) {
                const auto parsed = std::stoul(argv[argument_index + 3]);
                if (parsed == 0UL || parsed > 1'000UL) {
                    std::cerr << "MAX_RESULTS must be in the closed range 1..1000\n";
                    return 2;
                }
                maximum = static_cast<std::uint32_t>(parsed);
            }
            std::vector<fileman::orchestrator::SearchExactFilter> filters;
            filters.push_back({argv[argument_index + 1],
                               argv[argument_index + 2]});
            const auto page = client.search_subtree(
                argv[argument_index], std::nullopt, {}, maximum,
                std::nullopt, std::move(filters));
            std::cout << "terminal=" << page.terminal
                      << " source=" << page.source
                      << " complete=" << (page.complete ? "true" : "false")
                      << " generation=";
            if (page.generation) {
                std::cout << *page.generation;
            } else {
                std::cout << "none";
            }
            std::cout << " results=" << page.names.size();
            if (!page.names.empty()) std::cout << " first=" << page.names.front();
            if (page.cursor) {
                std::cout << " cursor_source=" << page.cursor->source
                          << " cursor=" << page.cursor->value;
            }
            std::cout << '\n';
        } else if (command == "shutdown") {
            if (argument_index != argc) {
                std::cerr << "shutdown accepts no arguments\n";
                return 2;
            }
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
