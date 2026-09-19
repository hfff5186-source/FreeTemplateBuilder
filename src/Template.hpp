#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

struct TemplateObject {
    int id = 0;
    float x = 0.f;
    float y = 0.f;
    float rotation = 0.f;
    float scaleX = 1.f;
    float scaleY = 1.f;
    int zOrder = 0;
};

struct TemplateData {
    std::string name;
    std::string algorithm = "PT";
    std::vector<TemplateObject> objects;
};

namespace TemplateIO {
    geode::Result<> save(TemplateData const&, std::string const&);
    geode::Result<TemplateData> load(std::string const&);
    geode::Result<std::vector<std::string>> list();
    geode::Result<> remove(std::string const&);
}
