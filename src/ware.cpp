#include "skopin/ware.hpp"

namespace skopin {

std::optional<std::string> validateWare(const Ware& ware) {
    if (ware.mark.empty()) {
        return std::string("empty mark");
    }
    for (char ch : ware.mark) {
        if (ch == ' ' || ch == '\t' || ch == '\n') {
            return std::string("bad mark");
        }
    }
    if (ware.height_cm <= 0 || ware.height_cm > kKilnHeightCm) {
        return std::string("bad height");
    }
    if (ware.thickness_mm <= 0) {
        return std::string("bad thickness");
    }
    return std::nullopt;
}

bool fitsInKiln(const std::vector<Ware>& stack, const Ware& next) {
    int total = next.height_cm + kShelfCm * static_cast<int>(stack.size());
    for (const Ware& ware : stack) {
        total += ware.height_cm;
    }
    return total <= kKilnHeightCm;
}

}  
