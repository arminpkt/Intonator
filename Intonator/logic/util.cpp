//
// Created by Vos on 28/10/2025.
//

#include <cmath>
#include <cassert>
#include <numeric>

#include "util.h"

#include <utility>


float dists_sum(const std::vector<PitchClass> &freq_as, const std::vector<PitchClass> &freq_bs) {
    assert(freq_as.size() == freq_bs.size() && "Vectors must have the same size");

    float sum = 0;
    for (size_t i = 0; i < freq_as.size(); i++) {
        sum += dist(freq_as[i], freq_bs[i]);
    }

    return sum;
}

void optimiseDestinationOrder(const std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered) {
    if (as_ordered.size() != bs_ordered.size())
        return;

    // vector of pairs of PitchClasses and their indices in as, sorted by PitchClass
    std::vector<std::pair<PitchClass, size_t>> as_pcs(as_ordered.size());
    for (size_t i = 0; i < as_ordered.size(); ++i)
        as_pcs[i] = std::make_pair(as_ordered[i]->getPitchClass(), i);
    std::sort(as_pcs.begin(), as_pcs.end(),
          [](auto const& a1, auto const& a2) {
              return a1.first < a2.first;
    });

    // vector of pairs of PitchClasses and their indices in bs, sorted by PitchClass
    std::vector<std::pair<PitchClass, size_t>> bs_pcs(bs_ordered.size());
    for (size_t i = 0; i < bs_ordered.size(); ++i)
        bs_pcs[i] = std::make_pair(bs_ordered[i]->getPitchClass(), i);
    std::sort(bs_pcs.begin(), bs_pcs.end(),
          [](auto const& b1, auto const& b2) {
              return b1.first < b2.first;
          });

    // finding the best rotation of bs to match as
    size_t best_rotation = 0;
    float best_score = std::numeric_limits<float>::infinity();

    std::vector<PitchClass> as_firsts, bs_firsts;
    as_firsts.reserve(as_ordered.size());
    for (const auto& [fst, snd] : as_pcs)
        as_firsts.push_back(fst);

    bs_firsts.reserve(bs_ordered.size());
    for (const auto& [fst, snd] : bs_pcs)
        bs_firsts.push_back(fst);

    // find best rotation
    for (size_t i = 0; i < bs_ordered.size(); ++i) {
        float score = dists_sum(as_firsts, bs_firsts);
        if (score < best_score) {
            best_score = score;
            best_rotation = i;
        }
        std::rotate(bs_firsts.begin(), bs_firsts.begin() + 1, bs_firsts.end());
    }

    // rotate bs_pcs according to the best rotation value found
    std::rotate(bs_pcs.begin(), bs_pcs.begin() + static_cast<std::vector<int>::difference_type>(best_rotation), bs_pcs.end());

    // create a new vector
    std::vector<Note*> bs_best_order(bs_firsts.size());
    for (size_t i = 0; i < as_ordered.size(); ++i)
        bs_best_order[as_pcs[i].second] = bs_ordered[bs_pcs[i].second];

    std::swap(bs_ordered, bs_best_order);
}

void optimiseOctaves(const std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered) {
    if (as_ordered.size() != bs_ordered.size())
        return;

    for (size_t i = 0; i < as_ordered.size(); ++i) {
        if (as_ordered[i] && bs_ordered[i])
            bs_ordered[i]->octavateClosestTo(*as_ordered[i]);
    }
}

void optimiseTransition(const std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered) {
    optimiseDestinationOrder(as_ordered, bs_ordered);
    optimiseOctaves(as_ordered, bs_ordered);
}
