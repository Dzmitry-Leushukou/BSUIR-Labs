#pragma once

#include "petri_model.hpp"

#include <cstdint>
#include <random>
#include <unordered_map>

struct ExperimentOptions {
    int runs = 10000;
    int maxSteps = 30;
    std::uint32_t seed = 17;
    PetriNet::Marking target;
};

struct ExactExperiment {
    std::vector<double> finalProbabilities;
    std::vector<double> transitionMeans;
    std::vector<double> finalTokenMeans;
    std::vector<int> reachableMaxTokens;
    double deadlockProbability = 0;
    double targetHitProbability = 0;
    double parallelFinalProbability = 0;
    double meanFirings = 0;
};

struct ExperimentSamples {
    int completedRuns = 0;
    std::vector<std::uint64_t> finalCounts;
    std::vector<std::uint64_t> transitionCounts;
    std::vector<int> observedMaxTokens;
    std::vector<bool> visited;
    std::uint64_t deadlockRuns = 0;
    std::uint64_t targetHitRuns = 0;
    std::uint64_t parallelFinalRuns = 0;
    std::uint64_t firings = 0;
};

// Each trial starts at M0. A uniformly selected enabled transition fires per step.
// A deadlock ends a trial; its marking remains the endpoint for the given horizon.
class PetriExperiment {
public:
    PetriExperiment(PetriNet net, ExperimentOptions options);
    void runBatch(int count);

    const PetriNet& net() const { return net_; }
    const ExperimentOptions& options() const { return options_; }
    const PetriNet::Reachability& graph() const { return graph_; }
    const ExactExperiment& exact() const { return exact_; }
    const ExperimentSamples& samples() const { return samples_; }
    bool finished() const { return samples_.completedRuns == options_.runs; }

private:
    struct Edge { int to; int transition; };
    PetriNet net_;
    ExperimentOptions options_;
    PetriNet::Reachability graph_;
    std::vector<std::vector<Edge>> outgoing_;
    std::vector<bool> parallel_;
    std::unordered_map<std::string, int> stateIds_;
    ExactExperiment exact_;
    ExperimentSamples samples_;
    std::mt19937 random_;
    int targetId_ = -1;

    void calculateExact();
    void runTrial();
};
