#include "petri_model.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <queue>
#include <set>
#include <sstream>
#include <unordered_map>

namespace {

std::string joinNames(const std::vector<std::string>& names) {
    std::ostringstream out;
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (i != 0) {
            out << ", ";
        }
        out << names[i];
    }
    return out.str();
}

std::string boolText(bool value) {
    return value ? "да" : "нет";
}

} // namespace

bool PetriNet::isEnabled(int transition, const Marking& state) const {
    if (transition < 0 || transition >= static_cast<int>(transitions.size()) ||
        state.size() != places.size()) {
        return false;
    }

    const auto& input = transitions[transition].pre;
    for (std::size_t place = 0; place < places.size(); ++place) {
        if (state[place] < input[place]) {
            return false;
        }
    }
    return true;
}

bool PetriNet::fire(int transition, Marking& state) const {
    if (!isEnabled(transition, state)) {
        return false;
    }

    const auto& current = transitions[transition];
    for (std::size_t place = 0; place < places.size(); ++place) {
        state[place] += current.post[place] - current.pre[place];
    }
    return true;
}

std::vector<int> PetriNet::enabledTransitions(const Marking& state) const {
    std::vector<int> result;
    for (int transition = 0; transition < static_cast<int>(transitions.size()); ++transition) {
        if (isEnabled(transition, state)) {
            result.push_back(transition);
        }
    }
    return result;
}

bool PetriNet::canFireTogether(const std::vector<int>& transitionIds, const Marking& state) const {
    if (transitionIds.size() < 2 || state.size() != places.size()) {
        return false;
    }

    std::vector<int> required(places.size(), 0);
    std::set<int> uniqueTransitions;
    for (const int transition : transitionIds) {
        if (transition < 0 || transition >= static_cast<int>(transitions.size()) ||
            !uniqueTransitions.insert(transition).second) {
            return false;
        }
        for (std::size_t place = 0; place < places.size(); ++place) {
            required[place] += transitions[transition].pre[place];
        }
    }

    for (std::size_t place = 0; place < places.size(); ++place) {
        if (state[place] < required[place]) {
            return false;
        }
    }
    return true;
}

std::vector<std::vector<int>> PetriNet::parallelSets(const Marking& state) const {
    std::vector<std::vector<int>> result;
    const auto count = transitions.size();
    if (count < 2 || count > 20) {
        return result;
    }

    const std::uint64_t combinations = std::uint64_t{1} << count;
    for (std::uint64_t mask = 1; mask < combinations; ++mask) {
        if (std::popcount(mask) < 2) {
            continue;
        }
        std::vector<int> candidate;
        for (std::size_t i = 0; i < count; ++i) {
            if ((mask & (std::uint64_t{1} << i)) != 0) {
                candidate.push_back(static_cast<int>(i));
            }
        }
        if (canFireTogether(candidate, state)) {
            result.push_back(candidate);
        }
    }
    return result;
}

PetriNet::Reachability PetriNet::buildReachability(std::size_t maxStates) const {
    Reachability result;
    if (initial.size() != places.size() || maxStates == 0) {
        result.complete = false;
        return result;
    }

    std::unordered_map<std::string, int> known;
    result.markings.push_back(initial);
    known.emplace(markingKey(initial), 0);

    for (std::size_t current = 0; current < result.markings.size(); ++current) {
        // Adding a newly discovered state may reallocate the vector.
        const Marking state = result.markings[current];
        for (int transition = 0; transition < static_cast<int>(transitions.size()); ++transition) {
            Marking next = state;
            if (!fire(transition, next)) {
                continue;
            }

            const auto key = markingKey(next);
            auto found = known.find(key);
            if (found == known.end()) {
                if (result.markings.size() >= maxStates) {
                    result.complete = false;
                    return result;
                }
                const int nextIndex = static_cast<int>(result.markings.size());
                known.emplace(key, nextIndex);
                result.markings.push_back(std::move(next));
                found = known.find(key);
            }
            result.edges.emplace_back(static_cast<int>(current), found->second);
        }
    }
    return result;
}

bool PetriNet::findPath(const Marking& target, std::vector<int>& path, std::size_t maxStates) const {
    path.clear();
    if (target.size() != places.size()) {
        return false;
    }

    struct Parent {
        int state = -1;
        int transition = -1;
    };

    std::unordered_map<std::string, int> known;
    std::vector<Marking> states;
    std::vector<Parent> parents;
    std::queue<int> pending;
    states.push_back(initial);
    parents.push_back({});
    known.emplace(markingKey(initial), 0);
    pending.push(0);

    int targetIndex = -1;
    while (!pending.empty() && states.size() <= maxStates) {
        const int current = pending.front();
        pending.pop();
        if (states[current] == target) {
            targetIndex = current;
            break;
        }

        for (int transition = 0; transition < static_cast<int>(transitions.size()); ++transition) {
            Marking next = states[current];
            if (!fire(transition, next)) {
                continue;
            }
            const auto key = markingKey(next);
            if (known.contains(key)) {
                continue;
            }
            const int nextIndex = static_cast<int>(states.size());
            known.emplace(key, nextIndex);
            states.push_back(std::move(next));
            parents.push_back({current, transition});
            pending.push(nextIndex);
        }
    }

    if (targetIndex < 0) {
        return false;
    }
    while (parents[targetIndex].state >= 0) {
        path.push_back(parents[targetIndex].transition);
        targetIndex = parents[targetIndex].state;
    }
    std::reverse(path.begin(), path.end());
    return true;
}

bool PetriNet::transitionCanEventuallyFire(const Reachability& graph, int start, int transition) const {
    std::queue<int> pending;
    std::vector<bool> visited(graph.markings.size(), false);
    pending.push(start);
    visited[start] = true;

    while (!pending.empty()) {
        const int current = pending.front();
        pending.pop();
        if (isEnabled(transition, graph.markings[current])) {
            return true;
        }
        for (const auto& edge : graph.edges) {
            if (edge.first == current && !visited[edge.second]) {
                visited[edge.second] = true;
                pending.push(edge.second);
            }
        }
    }
    return false;
}

std::string PetriNet::analysis() const {
    std::ostringstream out;
    const auto graph = buildReachability();
    const auto enabled = enabledTransitions(marking);

    std::size_t arcs = 0;
    bool ordinary = true;
    for (const auto& transition : transitions) {
        for (std::size_t place = 0; place < places.size(); ++place) {
            arcs += (transition.pre[place] != 0) + (transition.post[place] != 0);
            ordinary = ordinary && transition.pre[place] <= 1 && transition.post[place] <= 1;
        }
    }

    bool markedGraph = true;
    bool conflictFree = true;
    for (std::size_t place = 0; place < places.size(); ++place) {
        int inputCount = 0;
        int outputCount = 0;
        for (const auto& transition : transitions) {
            inputCount += transition.post[place] != 0;
            outputCount += transition.pre[place] != 0;
        }
        markedGraph = markedGraph && inputCount == 1 && outputCount == 1;
        conflictFree = conflictFree && outputCount <= 1;
    }

    bool stateMachine = true;
    for (const auto& transition : transitions) {
        const int inputs = std::count_if(transition.pre.begin(), transition.pre.end(), [](int value) { return value != 0; });
        const int outputs = std::count_if(transition.post.begin(), transition.post.end(), [](int value) { return value != 0; });
        stateMachine = stateMachine && inputs == 1 && outputs == 1;
    }

    bool freeChoice = true;
    for (std::size_t place = 0; place < places.size(); ++place) {
        std::vector<int> consumers;
        for (int transition = 0; transition < static_cast<int>(transitions.size()); ++transition) {
            if (transitions[transition].pre[place] != 0) {
                consumers.push_back(transition);
            }
        }
        for (std::size_t i = 0; i < consumers.size(); ++i) {
            for (std::size_t j = i + 1; j < consumers.size(); ++j) {
                freeChoice = freeChoice && transitions[consumers[i]].pre == transitions[consumers[j]].pre;
            }
        }
    }

    int maxTokens = 0;
    bool safe = graph.complete;
    for (const auto& state : graph.markings) {
        for (const int tokens : state) {
            maxTokens = std::max(maxTokens, tokens);
            safe = safe && tokens <= 1;
        }
    }

    bool live = graph.complete;
    if (live) {
        for (int state = 0; state < static_cast<int>(graph.markings.size()) && live; ++state) {
            for (int transition = 0; transition < static_cast<int>(transitions.size()); ++transition) {
                live = transitionCanEventuallyFire(graph, state, transition);
                if (!live) {
                    break;
                }
            }
        }
    }

    out << "Сеть: " << title << "\n"
        << "Позиции: " << places.size() << " (" << joinNames(places) << ")\n"
        << "Переходы: " << transitions.size() << "\n"
        << "Дуги: " << arcs << "\n"
        << "Начальная маркировка: " << formatMarking(initial) << "\n"
        << "Текущая маркировка: " << formatMarking(marking) << "\n"
        << "Доступны сейчас: ";
    if (enabled.empty()) {
        out << "нет";
    } else {
        for (std::size_t i = 0; i < enabled.size(); ++i) {
            if (i != 0) out << ", ";
            out << transitions[enabled[i]].name;
        }
    }
    out << "\n\nТеоретические признаки:\n"
        << "Обычная сеть (веса дуг 1): " << boolText(ordinary) << "\n"
        << "Граф разметки (marked graph): " << boolText(markedGraph) << "\n"
        << "Сеть состояний (state machine): " << boolText(stateMachine) << "\n"
        << "Свободный выбор (free-choice): " << boolText(freeChoice) << "\n"
        << "Бесконфликтная: " << boolText(conflictFree) << "\n"
        << "Живая в исследованном графе: " << boolText(live) << "\n"
        << "Число достижимых маркировок: " << graph.markings.size();

    if (graph.complete) {
        out << "\nОграниченность: " << maxTokens << "-ограниченная"
            << "\nБезопасность (1-ограниченность): " << boolText(safe);
    } else {
        out << "\nГраф достижимости ограничен лимитом состояний; ограниченность не установлена.";
    }
    return out.str();
}

std::string PetriNet::graphDescription(std::size_t maxStates) const {
    const auto graph = buildReachability(maxStates);
    std::ostringstream out;
    out << "Граф достижимых маркировок\n"
        << "Состояний: " << graph.markings.size() << "\n";
    if (!graph.complete) {
        out << "Внимание: достигнут лимит состояний " << maxStates << ".\n";
    }
    for (std::size_t i = 0; i < graph.markings.size(); ++i) {
        out << "M" << i << " = " << formatMarking(graph.markings[i]) << "\n";
    }
    out << "\nПереходы:\n";
    for (const auto& edge : graph.edges) {
        const auto& from = graph.markings[edge.first];
        const auto& to = graph.markings[edge.second];
        for (int transition = 0; transition < static_cast<int>(transitions.size()); ++transition) {
            Marking candidate = from;
            if (fire(transition, candidate) && candidate == to) {
                out << "M" << edge.first << " --" << transitions[transition].name
                    << "--> M" << edge.second << "\n";
                break;
            }
        }
    }
    return out.str();
}

std::string PetriNet::formatMarking(const Marking& state) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < state.size(); ++i) {
        if (i != 0) out << ", ";
        out << state[i];
    }
    out << "]";
    return out.str();
}

bool PetriNet::parseMarking(const std::string& text, std::size_t expectedSize,
                            Marking& result, std::string& error) {
    result.clear();
    std::string normalized = text;
    for (char& character : normalized) {
        if (character == '[' || character == ']' || character == ';') {
            character = ' ';
        }
        if (character == ',') {
            character = ' ';
        }
    }

    std::istringstream input(normalized);
    int value = 0;
    while (input >> value) {
        if (value < 0) {
            error = "Количество фишек не может быть отрицательным.";
            return false;
        }
        result.push_back(value);
    }
    if (result.size() != expectedSize) {
        error = "Нужно указать ровно " + std::to_string(expectedSize) + " целых чисел.";
        result.clear();
        return false;
    }
    return true;
}

std::string PetriNet::markingKey(const Marking& state) {
    std::ostringstream out;
    for (const int value : state) {
        out << value << ',';
    }
    return out.str();
}

PetriNet makeLeftNetwork() {
    PetriNet net;
    net.title = "Схема 1: p1...p5, a...d";
    net.places = {"p1", "p2", "p3", "p4", "p5"};
    net.initial = {0, 0, 1, 1, 0};
    net.marking = net.initial;
    net.transitions = {
        {"a", {0, 0, 0, 1, 0}, {1, 0, 0, 0, 0}},
        {"b", {0, 0, 1, 0, 0}, {0, 1, 0, 0, 0}},
        {"c", {1, 1, 0, 0, 0}, {0, 0, 1, 1, 0}},
        {"d", {0, 0, 0, 1, 0}, {0, 0, 0, 0, 1}},
    };
    return net;
}

PetriNet makeRightNetwork() {
    PetriNet net;
    net.title = "Схема 2: p1...p6, t1...t4";
    net.places = {"p1", "p2", "p3", "p4", "p5", "p6"};
    net.initial = {1, 0, 0, 0, 0, 1};
    net.marking = net.initial;
    net.transitions = {
        {"t1", {1, 0, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 1}},
        {"t2", {0, 1, 0, 0, 0, 0}, {0, 0, 0, 1, 0, 0}},
        {"t3", {0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 1, 0}},
        {"t4", {0, 0, 0, 1, 1, 1}, {1, 0, 0, 0, 0, 0}},
    };
    return net;
}
