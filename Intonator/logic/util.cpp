//
// Created by Vos on 28/10/2025.
//

#include "util.h"

#include <cmath>
#include <cassert>
#include <numeric>

float dist(const Note& a, const Note& b) {
    float ratio = b.frequency / a.frequency;
    float ratio_log = std::log2(ratio);
    float reduced = ratio_log - std::round(ratio_log);
    return reduced*reduced;
}

std::vector<float> dists(const std::vector<Note*>& as, const std::vector<Note*>& bs) {
    assert(as.size() == bs.size() && "Vectors must have the same size");
    std::vector<float> dists;
    dists.reserve(as.size());
    for (size_t i = 0; i < as.size(); i++) {
        dists.push_back(dist(*as[i], *bs[i]));
    }
    return dists;
}

void optimiseDestinationOrder(const std::vector<Note>& as, std::vector<Note>& bs) {
    assert(as.size() == bs.size() && "Vectors must have the same size");
    std::vector<const Note*> as_ptrs;
    std::vector<Note*> bs_ptrs;
    as_ptrs.reserve(as.size());
    bs_ptrs.reserve(bs.size());
    for (auto& a : as)
        as_ptrs.push_back(&a);
    for (auto& b : bs)
        bs_ptrs.push_back(&b);
    optimiseDestinationOrder(reinterpret_cast<const std::vector<Note*>&>(as_ptrs), bs_ptrs);
}

void optimiseDestinationOrder(const std::vector<Note*>& as, std::vector<Note*>& bs) {
    assert(as.size() == bs.size() && "Vectors must have the same size");

    std::vector<Note*> permuted_bs = bs;
    std::sort(permuted_bs.begin(), permuted_bs.end());

    std::vector<float> best_dists = dists(as, permuted_bs);
    float best_total = std::accumulate(best_dists.begin(), best_dists.end(), 0.0f);
    do {
        std::vector<float> current_dists = dists(as, permuted_bs);
        float current_total = std::accumulate(current_dists.begin(), current_dists.end(), 0.0f);
        if (current_total < best_total) {
            best_total = current_total;
            bs = permuted_bs;
        }
    } while (std::next_permutation(permuted_bs.begin(), permuted_bs.end()));
}