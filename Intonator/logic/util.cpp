//
// Created by Vos on 28/10/2025.
//

#include <cmath>
#include <cassert>
#include <numeric>

#include "util.h"


float dists_sum(const std::vector<PitchClass> &freq_as, const std::vector<PitchClass> &freq_bs) {
    assert(freq_as.size() == freq_bs.size() && "Vectors must have the same size");

    float sum = 0;
    for (size_t i = 0; i < freq_as.size(); i++) {
        sum += dist(freq_as[i], freq_bs[i]);
    }

    return sum;
}

void optimiseDestinationOrder(const std::vector<Note*>& as, std::vector<Note*>& bs) {
    assert(as.size() == bs.size() && "Vectors must have the same size");

    // vector of pairs of PitchClasses and their indices in as, sorted by PitchClass
    std::vector<std::pair<PitchClass, size_t>> as_pcs(as.size());
    for (size_t i = 0; i < as.size(); ++i)
        as_pcs[i] = std::make_pair(as[i]->getPitchClass(), i);
    std::sort(as_pcs.begin(), as_pcs.end(),
          [](auto const& a1, auto const& a2) {
              return a1.second < a2.second;
    });

    // vector of pairs of PitchClasses and their indices in bs, sorted by PitchClass
    std::vector<std::pair<PitchClass, size_t>> bs_pcs(bs.size());
    for (size_t i = 0; i < bs.size(); ++i)
        bs_pcs[i] = std::make_pair(bs[i]->getPitchClass(), i);
    std::sort(bs_pcs.begin(), bs_pcs.end(),
          [](auto const& b1, auto const& b2) {
              return b1.second < b2.second;
          });

    // finding the best rotation of bs to match as
    size_t best_rotation = 0;
    float best_score = std::numeric_limits<float>::infinity();

    std::vector<PitchClass> as_firsts, bs_firsts;
    as_firsts.reserve(as.size());
    for (const auto& [fst, snd] : as_pcs)
        as_firsts.push_back(fst);

    bs_firsts.reserve(bs.size());
    for (const auto& [fst, snd] : bs_pcs)
        bs_firsts.push_back(fst);

    for (size_t i = 0; i < bs.size(); ++i) {
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
    for (size_t i = 0; i < as.size(); ++i)
        bs_best_order[as_pcs[i].second] = std::move(bs[bs_pcs[i].second]);

    std::swap(bs, bs_best_order);
}

void optimiseOctaves(const std::vector<Note*>& as, std::vector<Note*>& bs) {
    assert(as.size() == bs.size() && "Vectors must have the same size");

    for (size_t i = 0; i < as.size(); ++i) {
        bs[i]->octavateClosestTo(*as[i]);
    }
}