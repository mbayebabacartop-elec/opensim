#include "openlab/core/project_store.hpp"
#include "openlab/core/simulation_time.hpp"
#include "openlab/core/version.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace openlab::core;
namespace { int fail(const char* message) { std::cerr << message << '\n'; return 1; } }

int main() {
    if (version() != "0.1.0") return fail("Unexpected core version");
    auto ohm = si::resistance.multipliedBy(si::electricCurrent);
    if (!ohm || ohm.value() != si::voltage) return fail("SI dimension product failed");

    ComponentDefinition source{{"test.source", "1.0"}, "Source", ScientificDomain::Electrical,
        {{"p", "Positive", ScientificDomain::Electrical, PortKind::Conserving, SignalDirection::Bidirectional, {}, si::voltage, si::electricCurrent}},
        {{"voltage", "Voltage", si::voltage, 12.0, 0.0, 24.0}}};
    ComponentDefinition load{{"test.load", "1.0"}, "Load", ScientificDomain::Electrical,
        {{"p", "Terminal", ScientificDomain::Electrical, PortKind::Conserving, SignalDirection::Bidirectional, {}, si::voltage, si::electricCurrent}}, {}};
    ProjectModel project("project.test", "Core round trip");
    if (!project.addDefinition(source) || !project.addDefinition(load)) return fail("Definition validation failed");
    if (!project.addComponent({"source-1", source.ref, "Source", {}}) || !project.addComponent({"load-1", load.ref, "Load", {}})) return fail("Component insertion failed");
    if (!project.setParameter("source-1", "voltage", {18.0, si::voltage})) return fail("Parameter setting failed");
    if (!project.connect({"source-1", "p"}, {"load-1", "p"})) return fail("Conserving-port connection failed");

    EventQueue queue;
    if (!queue.schedule({0.2, EventKind::InputUpdate, "x", "value", {2.0, si::dimensionless}}, 0.0) || !queue.schedule({0.1, EventKind::InputUpdate, "x", "value", {1.0, si::dimensionless}}, 0.0)) return fail("Event scheduling failed");
    auto due = queue.popDue(0.15);
    if (!due || due.value().size() != 1 || due.value().front().timeSeconds != 0.1) return fail("Event queue ordering failed");

    const auto path = std::filesystem::temp_directory_path() / "openlab-core-roundtrip.olsim";
    auto saved = ProjectStore::save(project, path);
    if (!saved) { std::cerr << saved.error().message << '\n'; return fail("Project save failed"); }
    auto loaded = ProjectStore::load(path);
    std::error_code ec; std::filesystem::remove(path, ec);
    if (!loaded) { std::cerr << loaded.error().message << '\n'; return fail("Project load failed"); }
    if (loaded.value().id() != project.id() || loaded.value().components().size() != 2 || loaded.value().connections().size() != 1 || loaded.value().components().at("source-1").parameters.at("voltage").si_value != 18.0) return fail("Project round trip mismatch");
    {
        std::ofstream corrupt(path, std::ios::binary | std::ios::trunc);
        corrupt << "not a zip archive";
    }
    auto rejected = ProjectStore::load(path);
    std::filesystem::remove(path, ec);
    if (rejected || rejected.error().code != ErrorCode::CorruptData) return fail("Malformed archive was not rejected");
    return 0;
}
