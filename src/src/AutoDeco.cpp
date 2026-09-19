#include "AutoDeco.hpp"
#include <unordered_map>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
// NOTE ON OBJECT IDs
// ---------------------------------------------------------------------------
// The IDs below are placeholders (kDECO_* constants). They must be filled in
// with real Geometry Dash base-game decoration object IDs before shipping.
// Do NOT pull these from another mod's binary/resources — cross-check them
// yourself in the in-game editor (build menu -> hover an object -> ID shown
// in Geode's "Enable Move Trigger / Show object IDs" dev tool, or via
// geode.node-ids / community object-ID spreadsheets for the *base game*).
// Every ID here refers to a stock object that ships with Geometry Dash
// itself, not to any third-party mod's content.
// ---------------------------------------------------------------------------

namespace {

// Grass theme
constexpr int kDECO_GrassShort   = 1;  // TODO: verify real ID
constexpr int kDECO_GrassTall    = 2;  // TODO: verify real ID
constexpr int kDECO_Bush         = 3;  // TODO: verify real ID
constexpr int kDECO_Flower       = 4;  // TODO: verify real ID
constexpr int kDECO_TreeSmall    = 5;  // TODO: verify real ID
constexpr int kDECO_TreeTall     = 6;  // TODO: verify real ID

// Rock/cave theme
constexpr int kDECO_RockSmall    = 7;  // TODO: verify real ID
constexpr int kDECO_RockLarge    = 8;  // TODO: verify real ID
constexpr int kDECO_Stalactite   = 9;  // TODO: verify real ID
constexpr int kDECO_Rubble       = 10; // TODO: verify real ID

// Tech theme
constexpr int kDECO_PipeSeg      = 11; // TODO: verify real ID
constexpr int kDECO_PanelSmall   = 12; // TODO: verify real ID
constexpr int kDECO_Vent         = 13; // TODO: verify real ID
constexpr int kDECO_Wire         = 14; // TODO: verify real ID

std::vector<DecoTheme>& themeRegistry() {
    static std::vector<DecoTheme> reg;
    return reg;
}

} // namespace

void AutoDeco::registerBuiltinThemes() {
    auto& reg = themeRegistry();
    if (!reg.empty())
        return; // already registered

    // ---- Grass theme ------------------------------------------------
    {
        DecoTheme grass;
        grass.name = "grass";
        grass.pieces = {
            /*0*/ { kDECO_GrassShort, 3.0f, 0.85f, 1.15f, true,  {1, 3} },
            /*1*/ { kDECO_GrassTall,  1.6f, 0.9f,  1.2f,  true,  {0, 3} },
            /*2*/ { kDECO_Bush,       1.0f, 0.8f,  1.3f,  true,  {0}    },
            /*3*/ { kDECO_Flower,     0.7f, 0.7f,  1.0f,  true,  {0}    },
            /*4*/ { kDECO_TreeSmall,  0.4f, 0.9f,  1.2f,  true,  {2}    },
            /*5*/ { kDECO_TreeTall,   0.15f,0.95f, 1.15f, true,  {4}    },
        };
        grass.layers = {
            { DecoLayer::Background, 6.0f, 0.6f, 140.f, -3 },
            { DecoLayer::Midground,  3.5f, 0.85f, 210.f, -1 },
            { DecoLayer::Foreground, 1.2f, 1.05f, 255.f,  1 },
        };
        reg.push_back(std::move(grass));
    }

    // ---- Rock / cave theme ------------------------------------------
    {
        DecoTheme rock;
        rock.name = "rock";
        rock.pieces = {
            /*0*/ { kDECO_RockSmall,  3.0f, 0.8f, 1.2f,  true,  {1} },
            /*1*/ { kDECO_RockLarge,  1.0f, 0.9f, 1.4f,  true,  {0} },
            /*2*/ { kDECO_Stalactite, 1.2f, 0.85f,1.25f, false, {}  },
            /*3*/ { kDECO_Rubble,     1.8f, 0.7f, 1.1f,  true,  {0} },
        };
        rock.layers = {
            { DecoLayer::Background, 5.0f, 0.6f, 120.f, -3 },
            { DecoLayer::Midground,  2.8f, 0.85f,190.f, -1 },
            { DecoLayer::Foreground, 0.9f, 1.0f, 255.f,  1 },
        };
        reg.push_back(std::move(rock));
    }

    // ---- Tech theme ---------------------------------------------------
    {
        DecoTheme tech;
        tech.name = "tech";
        tech.pieces = {
            /*0*/ { kDECO_PipeSeg,    2.2f, 0.9f, 1.1f, false, {2}    },
            /*1*/ { kDECO_PanelSmall, 2.5f, 0.85f,1.15f,false, {}     },
            /*2*/ { kDECO_Vent,       1.0f, 0.9f, 1.1f, false, {0}    },
            /*3*/ { kDECO_Wire,       1.6f, 0.95f,1.05f,true,  {0,2}  },
        };
        tech.layers = {
            { DecoLayer::Background, 4.5f, 0.65f,150.f, -3 },
            { DecoLayer::Midground,  2.6f, 0.9f, 210.f, -1 },
            { DecoLayer::Foreground, 0.7f, 1.0f, 255.f,  1 },
        };
        reg.push_back(std::move(tech));
    }
}

std::vector<std::string> AutoDeco::availableThemes() {
    std::vector<std::string> out;
    for (auto& t : themeRegistry())
        out.push_back(t.name);
    return out;
}

DecoTheme const* AutoDeco::findTheme(std::string const& name) {
    for (auto& t : themeRegistry())
        if (t.name == name)
            return &t;
    return nullptr;
}

// ---------------------------------------------------------------------------
// Ground scanning
// ---------------------------------------------------------------------------
// Walks the editor's placed objects, buckets them into 30-unit X columns,
// and finds the highest solid block in each column to build a ground
// heightmap. Contiguous columns with no solid block become "gap" segments
// (which the generator skips), so decoration never floats over holes.
std::vector<GroundSegment> AutoDeco::scanGround(
    cocos2d::CCArray* levelObjects,
    float xStart,
    float xEnd
) {
    constexpr float kColumnWidth = 30.f;
    std::vector<GroundSegment> segments;

    if (!levelObjects || xEnd <= xStart)
        return segments;

    int columnCount = static_cast<int>(std::ceil((xEnd - xStart) / kColumnWidth)) + 1;
    std::vector<float> columnTopY(columnCount, -1.f); // -1 = no ground found

    // NOTE: this assumes GameObject exposes getPositionX/Y and a way to
    // check "is this a solid ground block" (e.g. via its object ID belonging
    // to the ground-block ID set, or CCNode bounding box). The exact
    // GameObject accessor differs slightly between Geode binding releases,
    // so it's isolated here the same way EditorCapture.cpp isolates its
    // binding-specific calls.
    CCARRAY_FOREACH_B_TYPE(levelObjects, obj, GameObject) {
        if (!obj) continue;
        float px = obj->getPositionX();
        float py = obj->getPositionY();
        if (px < xStart || px > xEnd) continue;

        // Placeholder solidity check -- replace with the real "is ground
        // block" predicate for the installed bindings.
        bool isSolidGround = obj->m_objectType == GameObjectType::Solid;
        if (!isSolidGround) continue;

        int col = static_cast<int>((px - xStart) / kColumnWidth);
        col = std::clamp(col, 0, columnCount - 1);
        columnTopY[col] = std::max(columnTopY[col], py);
    }

    // Collapse the heightmap into contiguous segments (ground vs gap).
    int i = 0;
    while (i < columnCount) {
        bool gap = columnTopY[i] < 0.f;
        int j = i;
        float avgY = 0.f;
        int n = 0;
        while (j < columnCount && (columnTopY[j] < 0.f) == gap) {
            if (!gap) { avgY += columnTopY[j]; n++; }
            j++;
        }
        GroundSegment seg;
        seg.startX = xStart + i * kColumnWidth;
        seg.endX   = xStart + j * kColumnWidth;
        seg.isGap  = gap;
        seg.groundY = (n > 0) ? (avgY / n) : 0.f;
        segments.push_back(seg);
        i = j;
    }

    return segments;
}

// ---------------------------------------------------------------------------
// Generation
// ---------------------------------------------------------------------------
std::vector<TemplateObject> AutoDeco::generate(
    std::string const& themeName,
    std::vector<GroundSegment> const& segments,
    unsigned int seed
) {
    std::vector<TemplateObject> result;
    auto const* theme = findTheme(themeName);
    if (!theme) return result;

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> unit(0.f, 1.f);

    for (auto const& seg : segments) {
        if (seg.isGap) continue;
        float segLen = seg.endX - seg.startX;
        if (segLen <= 0.f) continue;

        for (auto const& layerCfg : theme->layers) {
            // Expected piece count for this layer over this segment,
            // sampled with a Poisson-ish approximation (uniform jitter is
            // sufficient here and keeps this dependency-free).
            float expected = layerCfg.density * (segLen / 100.f);
            int count = static_cast<int>(std::floor(expected));
            if (unit(rng) < (expected - count)) count++;

            // Track the last two chosen palette indices to avoid
            // three-in-a-row repeats, and the last placed piece to bias
            // clustering.
            int lastIdx = -1, secondLastIdx = -1;
            float lastX = -1e9f;

            for (int k = 0; k < count; ++k) {
                // Build a weighted candidate list, boosting pieces that
                // cluster with whatever was placed immediately before
                // (only if we're still "close" to it -> real clustering,
                // not just biasing the whole layer).
                std::vector<float> weights(theme->pieces.size());
                float total = 0.f;
                float posX = seg.startX + unit(rng) * segLen;
                bool nearLast = (lastIdx >= 0) && (std::abs(posX - lastX) < 60.f);

                for (size_t p = 0; p < theme->pieces.size(); ++p) {
                    float w = theme->pieces[p].baseWeight;
                    if (nearLast) {
                        auto const& likes = theme->pieces[lastIdx].clustersWith;
                        if (std::find(likes.begin(), likes.end(), (int)p) != likes.end())
                            w *= 2.4f; // cluster boost
                    }
                    // Discourage 3-in-a-row of the exact same piece.
                    if ((int)p == lastIdx && (int)p == secondLastIdx)
                        w *= 0.15f;
                    weights[p] = w;
                    total += w;
                }
                if (total <= 0.f) continue;

                float roll = unit(rng) * total;
                size_t chosen = 0;
                for (size_t p = 0; p < weights.size(); ++p) {
                    roll -= weights[p];
                    if (roll <= 0.f) { chosen = p; break; }
                }

                auto const& piece = theme->pieces[chosen];
                TemplateObject obj;
                obj.id = piece.objectId;
                obj.x = posX;
                obj.y = seg.groundY;
                obj.rotation = 0.f; // ground-aligned; adjust if slope data is added later
                float scale = piece.minScale + unit(rng) * (piece.maxScale - piece.minScale);
                obj.scaleX = piece.allowFlipX && unit(rng) < 0.5f ? -scale : scale;
                obj.scaleY = scale * layerCfg.scaleMul;
                obj.zOrder = layerCfg.zOrder;
                result.push_back(obj);

                secondLastIdx = lastIdx;
                lastIdx = (int)chosen;
                lastX = posX;
            }
        }
    }

    return result;
}
