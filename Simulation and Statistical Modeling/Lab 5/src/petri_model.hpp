#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

struct PetriTransition {
    std::string name;
    std::vector<int> pre;
    std::vector<int> post;
};

class PetriNet {
public:
    using Marking = std::vector<int>;

    struct Reachability {
        std::vector<Marking> markings;
        std::vector<std::pair<int, int>> edges;
        bool complete = true;
    };

    std::string title;
    std::vector<std::string> places;
    std::vector<PetriTransition> transitions;
    Marking initial;
    Marking marking;

    bool isEnabled(int transition, const Marking& state) const;
    bool fire(int transition, Marking& state) const;
    std::vector<int> enabledTransitions(const Marking& state) const;
    bool canFireTogether(const std::vector<int>& transitionIds, const Marking& state) const;
    std::vector<std::vector<int>> parallelSets(const Marking& state) const;

    Reachability buildReachability(std::size_t maxStates = 20000) const;
    bool findPath(const Marking& target, std::vector<int>& path, std::size_t maxStates = 20000) const;
    std::string analysis() const;
    std::string graphDescription(std::size_t maxStates = 20000) const;

    static std::string formatMarking(const Marking& state);
    static bool parseMarking(const std::string& text, std::size_t expectedSize,
                             Marking& result, std::string& error);

private:
    static std::string markingKey(const Marking& state);
    bool transitionCanEventuallyFire(const Reachability& graph, int start, int transition) const;
};

PetriNet makeLeftNetwork();
PetriNet makeRightNetwork();
