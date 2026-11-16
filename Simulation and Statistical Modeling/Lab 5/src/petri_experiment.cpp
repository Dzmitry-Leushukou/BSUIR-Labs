#include "petri_experiment.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

PetriExperiment::PetriExperiment(PetriNet net, ExperimentOptions options)
    : net_(std::move(net)), options_(std::move(options)), random_(options_.seed) {
    if (options_.runs < 1 || options_.runs > 1000000 || options_.maxSteps < 0 ||
        options_.maxSteps > 100000 ||
        static_cast<std::int64_t>(options_.runs) * options_.maxSteps > 5000000) {
        throw std::invalid_argument("Не более 1 000 000 прогонов, 100 000 шагов и 5 000 000 событий в серии.");
    }
    if (options_.target.size() != net_.places.size() ||
        std::any_of(options_.target.begin(), options_.target.end(), [](int v) { return v < 0; })) {
        throw std::invalid_argument("Цель должна содержать неотрицательные числа для всех позиций.");
    }
    graph_ = net_.buildReachability();
    if (!graph_.complete || graph_.markings.empty()) {
        throw std::invalid_argument("Для точного сравнения нужен полный конечный граф маркировок.");
    }
    const auto states = graph_.markings.size();
    outgoing_.resize(states);
    parallel_.resize(states);
    for (int s = 0; s < static_cast<int>(states); ++s) {
        stateIds_.emplace(PetriNet::formatMarking(graph_.markings[s]), s);
        if (graph_.markings[s] == options_.target) targetId_ = s;
    }
    for (int s = 0; s < static_cast<int>(states); ++s) {
        for (const int t : net_.enabledTransitions(graph_.markings[s])) {
            auto next = graph_.markings[s];
            net_.fire(t, next);
            outgoing_[s].push_back({stateIds_.at(PetriNet::formatMarking(next)), t});
        }
        parallel_[s] = !net_.parallelSets(graph_.markings[s]).empty();
    }
    samples_.finalCounts.resize(states);
    samples_.transitionCounts.resize(net_.transitions.size());
    samples_.observedMaxTokens = net_.initial;
    samples_.visited.resize(states, false);
    calculateExact();
}

void PetriExperiment::calculateExact() {
    const auto size = graph_.markings.size();
    std::vector<double> distribution(size, 0), withoutTarget(size, 0);
    distribution[0] = 1;
    withoutTarget[0] = targetId_ == 0 ? 0 : 1;
    exact_.transitionMeans.assign(net_.transitions.size(), 0);
    exact_.reachableMaxTokens.assign(net_.places.size(), 0);
    exact_.finalTokenMeans.assign(net_.places.size(), 0);

    for (int step = 0; step < options_.maxSteps; ++step) {
        std::vector<double> next(size, 0), nextWithoutTarget(size, 0);
        for (int s = 0; s < static_cast<int>(size); ++s) {
            const auto& edges = outgoing_[s];
            if (edges.empty()) {
                // An absorbing self-loop is a probability bookkeeping step,
                // never a Petri transition or an additional firing.
                next[s] += distribution[s];
                nextWithoutTarget[s] += withoutTarget[s];
                continue;
            }
            const double probability = 1.0 / static_cast<double>(edges.size());
            for (const auto& edge : edges) {
                next[edge.to] += distribution[s] * probability;
                exact_.transitionMeans[edge.transition] += distribution[s] * probability;
                if (edge.to != targetId_) {
                    nextWithoutTarget[edge.to] += withoutTarget[s] * probability;
                }
            }
        }
        distribution = std::move(next);
        withoutTarget = std::move(nextWithoutTarget);
    }
    exact_.finalProbabilities = distribution;
    exact_.targetHitProbability = std::clamp(
        1.0 - std::accumulate(withoutTarget.begin(), withoutTarget.end(), 0.0), 0.0, 1.0);
    exact_.meanFirings = std::accumulate(exact_.transitionMeans.begin(), exact_.transitionMeans.end(), 0.0);
    for (int s = 0; s < static_cast<int>(size); ++s) {
        if (outgoing_[s].empty()) exact_.deadlockProbability += distribution[s];
        if (parallel_[s]) exact_.parallelFinalProbability += distribution[s];
        for (std::size_t p = 0; p < net_.places.size(); ++p) {
            exact_.reachableMaxTokens[p] = std::max(exact_.reachableMaxTokens[p], graph_.markings[s][p]);
            exact_.finalTokenMeans[p] += distribution[s] * graph_.markings[s][p];
        }
    }
}

void PetriExperiment::runBatch(int count) {
    const int remaining = options_.runs - samples_.completedRuns;
    for (int run = 0; run < std::min(std::max(count, 0), remaining); ++run) runTrial();
}

void PetriExperiment::runTrial() {
    auto marking = net_.initial;
    int state = 0;
    bool hit = marking == options_.target;
    samples_.visited[state] = true;
    for (int step = 0; step < options_.maxSteps; ++step) {
        // Simulate the Petri rules directly, independently of the probability DP.
        const auto enabled = net_.enabledTransitions(marking);
        if (enabled.empty()) break;
        std::uniform_int_distribution<int> choose(0, static_cast<int>(enabled.size()) - 1);
        const int transition = enabled[choose(random_)];
        net_.fire(transition, marking);
        ++samples_.transitionCounts[transition];
        ++samples_.firings;
        state = stateIds_.at(PetriNet::formatMarking(marking));
        samples_.visited[state] = true;
        hit = hit || marking == options_.target;
        for (std::size_t p = 0; p < marking.size(); ++p) {
            samples_.observedMaxTokens[p] = std::max(samples_.observedMaxTokens[p], marking[p]);
        }
    }
    ++samples_.finalCounts[state];
    if (outgoing_[state].empty()) ++samples_.deadlockRuns;
    if (hit) ++samples_.targetHitRuns;
    if (parallel_[state]) ++samples_.parallelFinalRuns;
    ++samples_.completedRuns;
}
