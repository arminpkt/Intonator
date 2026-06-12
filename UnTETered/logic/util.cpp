//
// Created by Vos on 28/10/2025.
//

#include <cmath>
#include <cassert>
#include <numeric>

#include "util.h"


double distsSumSq(const std::vector<PitchClass> &freq_as, const std::vector<PitchClass> &freq_bs) {
    assert(freq_as.size() == freq_bs.size() && "Vectors must have the same size");

    double sum_sq = 0;
    for (size_t i = 0; i < freq_as.size(); i++) {
        auto d = dist(freq_as[i], freq_bs[i]);
        sum_sq += d * d;
    }

    return sum_sq;
}

double optimiseDestinationOrder(std::vector<Note*>& asOrdered, std::vector<Note*>& bsOrdered) {
    if (asOrdered.size() < bsOrdered.size()) {
        std::vector<Note*> asBestExtension;
        double asBestExtensionScore = std::numeric_limits<double>::infinity();
        for (auto pos = asOrdered.begin(); pos != asOrdered.end() + 1; ++pos) {
            std::vector<Note*> asOrderedExtended = asOrdered;
            asOrderedExtended.insert(pos, nullptr);
            double score = optimiseDestinationOrder(asOrderedExtended, bsOrdered);
            if (score < asBestExtensionScore) {
                asBestExtensionScore = score;
                asBestExtension = asOrderedExtended;
            }
        }

        std::swap(asOrdered, asBestExtension);
        return asBestExtensionScore;
    }

    if (bsOrdered.size() < asOrdered.size()) {
        std::vector<Note*> bsBestExtension;
        double bsBestExtensionScore = std::numeric_limits<double>::infinity();
        for (auto pos = bsOrdered.begin(); pos != bsOrdered.end() + 1; ++pos) {
            std::vector<Note*> bsOrderedExtended = bsOrdered;
            bsOrderedExtended.insert(pos, nullptr);
            double score = optimiseDestinationOrder(asOrdered, bsOrderedExtended);
            if (score < bsBestExtensionScore) {
                bsBestExtensionScore = score;
                bsBestExtension = bsOrderedExtended;
            }
        }

        std::swap(bsOrdered, bsBestExtension);
        return bsBestExtensionScore;
    }

    // vector of pairs of PitchClasses and their indices in `as`, sorted by PitchClass
    std::vector<std::optional<std::pair<PitchClass, size_t>>> asPcs(asOrdered.size());
    for (size_t i = 0; i < asOrdered.size(); ++i)
        if (asOrdered[i])
            asPcs[i] = std::make_pair(asOrdered[i]->getPitchClass(), i);
        else
            asPcs[i] = std::nullopt;
    std::sort(asPcs.begin(), asPcs.end(),
          [](auto const& a1, auto const& a2) {
              if (!a1)
                  return true;
              if (!a2)
                  return false;
              return a1.value().first < a2.value().first;
    });

    // vector of pairs of PitchClasses and their indices in `bs`, sorted by PitchClass
    std::vector<std::pair<PitchClass, size_t>> bsPcs(bsOrdered.size());
    for (size_t i = 0; i < bsOrdered.size(); ++i)
        bsPcs[i] = std::make_pair(bsOrdered[i]->getPitchClass(), i);
    std::sort(bsPcs.begin(), bsPcs.end(),
          [](auto const& b1, auto const& b2) {
              return b1.first < b2.first;
          });

    // finding the best rotation of `bs` to match `as`
    size_t bestRotation = 0;
    double bestScore = std::numeric_limits<double>::infinity();

    std::vector<PitchClass> asFirsts, bsFirsts;
    asFirsts.reserve(asOrdered.size());
    for (const auto& [fst, snd] : asPcs)
        asFirsts.push_back(fst);

    bsFirsts.reserve(bsOrdered.size());
    for (const auto& [fst, snd] : bsPcs)
        bsFirsts.push_back(fst);

    // find best rotation
    for (size_t i = 0; i < bsOrdered.size(); ++i) {
        double score = distsSumSq(asFirsts, bsFirsts);
        if (score < bestScore) {
            bestScore = score;
            bestRotation = i;
        }
        std::rotate(bsFirsts.begin(), bsFirsts.begin() + 1, bsFirsts.end());
    }

    // rotate `bs_pcs` according to the best rotation value found
    std::rotate(bsPcs.begin(), bsPcs.begin() + static_cast<std::vector<int>::difference_type>(bestRotation), bsPcs.end());

    // create a new vector
    std::vector<Note*> bs_best_order(bsFirsts.size());
    for (size_t i = 0; i < asOrdered.size(); ++i)
        bs_best_order[asPcs[i].second] = bsOrdered[bsPcs[i].second];

    std::swap(bsOrdered, bs_best_order);

    return bestScore;
}

void optimiseOctaves(const std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered) {
    if (as_ordered.size() != bs_ordered.size())
        return;

    for (size_t i = 0; i < as_ordered.size(); ++i) {
        if (as_ordered[i] && bs_ordered[i])
            bs_ordered[i]->octavateClosestTo(*as_ordered[i]);
    }
}

void optimiseTransition(std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered) {
    optimiseDestinationOrder(as_ordered, bs_ordered);
    optimiseOctaves(as_ordered, bs_ordered);
}

std::optional<std::vector<int>> getIntRatios(const std::vector<Note*>& notes, bool oddified) {
    if (notes.empty())
        return std::nullopt;

    for (const auto& note : notes)
        if (!note->isFamiliarWith(notes[0]))
            return std::nullopt;

    std::vector<Fraction> ratios{};
    ratios.reserve(notes.size());
    for (auto& note : notes)
            ratios.push_back(note->ratio);

    return getIntRatios(ratios, oddified);
}

std::vector<int> getIntRatios(std::vector<Fraction> ratios, bool oddified) {
    if (oddified)
        for (auto & fraction : ratios)
            fraction = fraction.oddified();

    for (size_t i = 0; i < primes::PrimeCount; ++i) {
        int smallestPower = getSmallestPowerAtIndex(ratios, i);
        addToPowersAtIndex(ratios, -smallestPower, i);
    }
    std::vector<int> intRatios;
    intRatios.reserve(ratios.size());
    for (const auto& fraction : ratios)
        intRatios.push_back(fraction.getNumeratorAndDenominator().first);
    return intRatios;
}

int getSmallestPowerAtIndex(const std::vector<Fraction>& fractions, const size_t i) {
    int smallestPower = std::numeric_limits<int>::max();
    for (const auto& fraction : fractions)
        if (const int primePower = fraction.getMonzo().primePowers[i]; primePower < smallestPower)
            smallestPower = primePower;
    return smallestPower;
}

void addToPowersAtIndex(std::vector<Fraction>& fractions, int increment, size_t index) {
    Fraction toMultiply = Fraction{primes::Primes[index], 1} ^ increment;
    for (auto& fraction : fractions) {
        fraction = fraction * toMultiply;
    }
}

std::tuple<int, int, int, int, int, int> getTLBRWH(Rect r) {
    auto topLeft = r.getTopLeft();
    auto bottomRight = r.getBottomRight();
    return {
        topLeft.getY(),
        topLeft.getX(),
        bottomRight.getY(),
        bottomRight.getX(),
        r.getWidth(),
        r.getHeight()
    };
}

std::tuple<float, float, float, float, float, float> getTLBRWH(RectF r) {
    auto topLeft = r.getTopLeft();
    auto bottomRight = r.getBottomRight();
    return {
        topLeft.getY(),
        topLeft.getX(),
        bottomRight.getY(),
        bottomRight.getX(),
        r.getWidth(),
        r.getHeight()
    };
}

std::vector<std::vector<Note*>> divideIntoVoices(std::vector<Note*> notes) {
    if (notes.empty())
        return {};

    std::sort(notes.begin(), notes.end(), [](Note* a, Note* b) {return (a->start < b->start); });

    auto firstChord = findFirstChordForSorted(notes);
    std::vector<std::vector<Note*>> voices;
    voices.reserve(firstChord.size());
    for (auto& note : firstChord)
        voices.push_back({note});

    float lastStart = firstChord.back()->start;

    std::vector<Note*> nextChord;
    for (size_t i = voices.size(); i < notes.size(); i++) {
        if (notes[0]->start > lastStart + 0.001f) {
            updateVoices(voices, nextChord);
            nextChord.clear();
        }
    }

    return voices;
}

std::vector<Note*> findFirstChordForSorted(std::vector<Note*> notes) {
    std::vector<Note*> firstChord;
    float lastStart = notes[0]->start;

    for (auto& note : notes)
        if (std::abs(lastStart - note->start) < 0.001f)
            firstChord.push_back(note);

    return firstChord;
}

void updateVoices(std::vector<std::vector<Note*>>& voices, std::vector<Note*>& nextChord) {
    if (nextChord.empty())
        return;

    float startNextChord = nextChord.back()->start;
    std::vector<std::vector<Note*>*> availableVoices;
    std::vector<Note*> lastNotesOfAvailableVoices;

    for (auto& voice : voices) {
        if (voice.back()->end < startNextChord - 0.001f) {
            availableVoices.push_back(&voice);
            lastNotesOfAvailableVoices.push_back(voice.back());
        }
    }
}