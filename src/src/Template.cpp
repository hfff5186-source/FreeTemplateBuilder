#include "Template.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>

using namespace geode::prelude;

static std::filesystem::path templateDir() {
    auto p = Mod::get()->getSaveDir() / "templates";
    std::error_code ec;
    std::filesystem::create_directories(p, ec);
    return p;
}

static std::filesystem::path templateFile(std::string name) {
    std::string safe;
    for (char c : name) {
        if (std::isalnum((unsigned char)c) || c == '_' || c == '-')
            safe += c;
    }
    if (safe.empty()) safe = "template";
    return templateDir() / (safe + ".tblib");
}

geode::Result<> TemplateIO::save(TemplateData const& d, std::string const& name) {
    std::ofstream out(templateFile(name));
    if (!out) return Err("Cannot write template");
    out << "TBLIB1\n";
    out << "name=" << d.name << "\n";
    out << "algorithm=" << d.algorithm << "\n";
    out << "objects=" << d.objects.size() << "\n";

    for (auto const& o : d.objects)
        out << o.id << ' ' << o.x << ' ' << o.y << ' '
            << o.rotation << ' ' << o.scaleX << ' ' << o.scaleY
            << ' ' << o.zOrder << '\n';

    return Ok();
}

geode::Result<TemplateData> TemplateIO::load(std::string const& name) {
    std::ifstream in(templateFile(name));
    if (!in) return Err("Template not found");

    TemplateData d;
    std::string line;

    if (!std::getline(in, line) || line != "TBLIB1")
        return Err("Invalid TBLIB");

    size_t count = 0;

    while (std::getline(in, line)) {
        if (line.rfind("name=", 0) == 0)
            d.name = line.substr(5);
        else if (line.rfind("algorithm=", 0) == 0)
            d.algorithm = line.substr(10);
        else if (line.rfind("objects=", 0) == 0) {
            count = std::stoul(line.substr(8));
            break;
        }
    }

    for (size_t i = 0; i < count; ++i) {
        TemplateObject o;

        if (!(in >> o.id >> o.x >> o.y >> o.rotation >>
              o.scaleX >> o.scaleY >> o.zOrder))
            return Err("Invalid object data");

        d.objects.push_back(o);
    }

    return d;
}

geode::Result<std::vector<std::string>> TemplateIO::list() {
    std::vector<std::string> result;
    std::error_code ec;

    for (auto const& e : std::filesystem::directory_iterator(templateDir(), ec))
        if (e.path().extension() == ".tblib")
            result.push_back(e.path().stem().string());

    std::sort(result.begin(), result.end());
    return result;
}

geode::Result<> TemplateIO::remove(std::string const& name) {
    std::error_code ec;

    if (!std::filesystem::remove(templateFile(name), ec))
        return Err("Template not found");

    return Ok();
}
