#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <queue>
#include <random>
#include <vector>

struct ModelParameters {
    int machine_count = 8;
    double failure_rate = 0.2;
    double service_time = 1.0;
};

struct ModelResult {
    std::vector<double> probabilities;
    double mu = 0.0;
    double average_repaired = 0.0;
    double average_faulty = 0.0;
    double average_waiting = 0.0;
    double busy_probability = 0.0;
};

struct SimulationParameters {
    double observation_time = 100000.0;
    std::uint64_t seed = 42;
};

struct SimulationResult {
    ModelResult estimates;
    std::uint64_t failures = 0;
    std::uint64_t repairs = 0;
    std::uint64_t processed_events = 0;
    int initial_state = 0;
    int final_state = 0;
    double measured_time = 0.0;
};

ModelResult calculate_model(const ModelParameters& parameters);

// Each working machine has its own failure clock; one operator serves a FIFO queue.
// advance() limits work per call so that the GUI can redraw and handle cancellation.
class RepairSimulation {
public:
    RepairSimulation(const ModelParameters& model, const SimulationParameters& simulation);
    bool advance(std::size_t event_budget = 2000);
    double progress() const;
    std::uint64_t processed_events() const { return processed_events_; }
    SimulationResult result() const;

private:
    struct Event {
        double time;
        int machine;
        bool repair;
        bool operator>(const Event& other) const { return time > other.time; }
    };

    void schedule(int machine, bool repair, double rate);

    ModelParameters model_;
    SimulationParameters simulation_;
    std::mt19937_64 random_;
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> events_;
    std::queue<int> repair_queue_;
    std::vector<double> state_times_;
    double mu_ = 0.0;
    double time_ = 0.0;
    double end_time_ = 0.0;
    std::uint64_t failures_ = 0;
    std::uint64_t repairs_ = 0;
    std::uint64_t processed_events_ = 0;
    bool finished_ = false;
};
