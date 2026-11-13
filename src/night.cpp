#include "skopin/night.hpp"

#include "skopin/kiln.hpp"

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace skopin {

namespace {

using Batch = std::vector<Ware>;

std::vector<Batch> makeBatches(std::vector<Ware> waiting) {
    std::vector<Batch> batches;
    while (!waiting.empty()) {
        Batch batch{waiting.front()};
        std::vector<Ware> rest;
        const Ware& base = batch.front();

        if (base.kind == WareKind::Sagger) {
            rest.assign(waiting.begin() + 1, waiting.end());
        } else {
            bool skipped = false;  
            std::size_t index = 1;
            for (; index < waiting.size(); ++index) {
                const Ware& candidate = waiting[index];
                if (candidate.kind == WareKind::Sagger || candidate.program != base.program) {
                    break;
                }
                if (fitsInKiln(batch, candidate) && (!skipped || candidate.kind == WareKind::Rush)) {
                    batch.push_back(candidate);
                } else {
                    skipped = true;
                    rest.push_back(candidate);
                }
            }
            rest.insert(rest.end(), waiting.begin() + static_cast<std::ptrdiff_t>(index), waiting.end());
        }

        batches.push_back(std::move(batch));
        waiting = std::move(rest);
    }
    return batches;
}

void loadBatch(Kiln& kiln, const Batch& batch) {
    for (auto it = batch.rbegin(); it != batch.rend(); ++it) {
        kiln.load(*it);
    }
}

void unloadBatch(Kiln& kiln, const Batch& batch, std::vector<std::string>& passed) {
    for (const Ware& ware : batch) {
        kiln.unload(ware.mark);
        passed.push_back(ware.mark);
    }
}

void runBisque(Kiln& kiln, const Batch& batch, bool nextIsGlaze, std::vector<std::string>& passed) {
    if (kiln.door() == Door::Open) {
        kiln.close();
    }
    if (kiln.cone() != Cone::Cold) {
        kiln.cool();
    }
    kiln.open();
    loadBatch(kiln, batch);
    kiln.close();
    kiln.fire();
    kiln.open();
    unloadBatch(kiln, batch, passed);
    if (nextIsGlaze) {
        return;
    }
    kiln.close();
    kiln.cool();
}

void runGlaze(Kiln& kiln, const Batch& batch, std::vector<std::string>& passed) {
    if (kiln.door() == Door::Closed) {
        kiln.open();
    }
    loadBatch(kiln, batch);
    kiln.close();
    while (kiln.cone() != Cone::Glaze) {
        kiln.fire();
    }
    kiln.cool();
    kiln.cool();
    kiln.open();
    unloadBatch(kiln, batch, passed);
    kiln.close();
}

}  

NightReport runNightFire(const std::vector<Ware>& queue) {
    NightReport report;

    std::set<std::string> seenMarks;
    std::vector<Ware> waiting;
    for (const Ware& ware : queue) {
        if (!seenMarks.insert(ware.mark).second) {
            report.refused.push_back(Refusal{ware.mark, "duplicate"});
        } else if (validateWare(ware).has_value()) {
            report.refused.push_back(Refusal{ware.mark, "invalid"});
        } else if (ware.thickness_mm > kCrackThicknessMm) {
            report.refused.push_back(Refusal{ware.mark, "thick"});
        } else {
            waiting.push_back(ware);
        }
    }

    const std::vector<Batch> batches = makeBatches(std::move(waiting));

    Kiln kiln;
    for (std::size_t i = 0; i < batches.size(); ++i) {
        const Batch& batch = batches[i];
        if (batch.front().program == Program::Bisque) {
            const bool nextIsGlaze = i + 1 < batches.size() && batches[i + 1].front().program == Program::Glaze;
            runBisque(kiln, batch, nextIsGlaze, report.passed);
        } else {
            runGlaze(kiln, batch, report.passed);
        }
    }

    report.stokes = kiln.stokes();
    return report;
}

} 
