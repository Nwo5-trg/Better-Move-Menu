#include "Utils.hpp"

using namespace geode::prelude;

std::string ftofstr(float num, int rounding) {
    auto str = numToString(num, rounding);
    auto end = str.find_last_not_of('0');
    if (end != std::string::npos) str.erase(end + 1);
    if (!str.empty() && str.back() == '.') str.pop_back();
    return str;
}

std::unordered_set<int> parseIntStringToSet(const std::string& input) {
    std::unordered_set<int> out;
    auto split = string::split(input, ",");
    for (auto num : split) out.insert(std::strtol(num.c_str(), nullptr, 10));
    return out;
}