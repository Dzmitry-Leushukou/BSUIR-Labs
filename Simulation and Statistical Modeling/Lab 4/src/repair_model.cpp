#include "repair_model.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace {
double validate_model(const ModelParameters& model) {
    if (model.machine_count < 1 || model.machine_count > 1000) {
        throw std::invalid_argument("Число станков n должно быть от 1 до 1000.");
    }
    if (!std::isfinite(model.failure_rate) || model.failure_rate <= 0.0 ||
        !std::isfinite(model.service_time) || model.service_time <= 0.0) {
        throw std::invalid_argument("Интенсивность отказов и время ремонта должны быть положительными конечными числами.");
    }
    const double mu = 1.0 / model.service_time;
    if (!std::isfinite(mu) || mu <= 0.0) {
        throw std::invalid_argument("Слишком малое время ремонта: интенсивность выходит за числовой диапазон.");
    }
    return mu;
}

void calculate_time_averages(ModelResult& result) {
    for (std::size_t k = 0; k < result.probabilities.size(); ++k) {
        const double probability = result.probabilities[k];
        result.average_faulty += static_cast<double>(k) * probability;
        if (k > 0) {
            result.busy_probability += probability;
        }
        if (k > 1) {
            result.average_waiting += static_cast<double>(k - 1) * probability;
        }
    }
}
}

ModelResult calculate_model(const ModelParameters& parameters) {
    ModelResult result;
    result.mu = validate_model(parameters);
    const auto state_count = static_cast<std::size_t>(parameters.machine_count) + 1;
    result.probabilities.resize(state_count);
    std::vector<double> log_weights(state_count, 0.0);
    for (std::size_t k = 1; k < state_count; ++k) {
        // p[k] / p[k-1] = (n-k+1) * lambda / mu; only one repair is active.
        log_weights[k] = log_weights[k - 1]
            + std::log(static_cast<double>(state_count - k))
            + std::log(parameters.failure_rate) - std::log(result.mu);
    }
    const double max_weight = *std::max_element(log_weights.begin(), log_weights.end());
    for (std::size_t k = 0; k < state_count; ++k) {
        result.probabilities[k] = std::exp(log_weights[k] - max_weight);
    }
    const double sum = std::accumulate(result.probabilities.begin(), result.probabilities.end(), 0.0);
    for (double& probability : result.probabilities) {
        probability /= sum;
    }
    calculate_time_averages(result);
    result.average_repaired = result.mu * result.busy_probability;
    return result;
}

RepairSimulation::RepairSimulation(const ModelParameters& model, const SimulationParameters& simulation)
    : model_(model), simulation_(simulation), random_(simulation.seed), mu_(validate_model(model)) {
    if (!std::isfinite(simulation.observation_time) || simulation.observation_time <= 0.0) {
        throw std::invalid_argument("Время наблюдения должно быть положительным конечным числом.");
    }
    end_time_ = simulation.observation_time;
    state_times_.resize(static_cast<std::size_t>(model.machine_count) + 1, 0.0);
    // Start with all machines working and measure the entire trajectory from time zero.
    for (int machine = 0; machine < model.machine_count; ++machine) {
        schedule(machine, false, model.failure_rate);
    }
}

void RepairSimulation::schedule(int machine, bool repair, double rate) {
    std::exponential_distribution<double> distribution(rate);
    double delay = 0.0;
    while (delay == 0.0) {
        delay = distribution(random_);
    }
    const double event_time = time_ + delay;
    if (!(event_time > time_)) {
        throw std::runtime_error("Не хватает точности времени: уменьшите длительность или интенсивности.");
    }
    events_.push({event_time, machine, repair});
}

bool RepairSimulation::advance(std::size_t event_budget) {
    for (std::size_t i = 0; i < event_budget && !finished_; ++i) {
        const Event event = events_.top();
        const auto state = repair_queue_.size();
        const double interval_end = std::min(event.time, end_time_);
        // Clip the last interval at T; every interval from launch contributes to the estimates.
        state_times_[state] += interval_end - time_;
        time_ = interval_end;
        if (event.time >= end_time_) {
            finished_ = true;
            break;
        }

        events_.pop();
        ++processed_events_;
        if (event.repair) {
            repair_queue_.pop();
            ++repairs_;
            // The repaired machine works again and gets a new failure clock.
            schedule(event.machine, false, model_.failure_rate);
            if (!repair_queue_.empty()) {
                schedule(repair_queue_.front(), true, mu_);
            }
        } else {
            const bool operator_was_free = repair_queue_.empty();
            repair_queue_.push(event.machine);
            ++failures_;
            if (operator_was_free) {
                schedule(event.machine, true, mu_);
            }
        }
    }
    return finished_;
}

double RepairSimulation::progress() const {
    return time_ / end_time_;
}

SimulationResult RepairSimulation::result() const {
    if (!finished_) {
        throw std::logic_error("Имитация ещё не завершена.");
    }
    SimulationResult result;
    result.estimates.mu = mu_;
    result.estimates.probabilities = state_times_;
    for (double& time : result.estimates.probabilities) {
        time /= simulation_.observation_time;
    }
    calculate_time_averages(result.estimates);
    // Estimate throughput from actual completed repairs, independently of theoretical mu * Pbusy.
    result.estimates.average_repaired = static_cast<double>(repairs_) / simulation_.observation_time;
    result.failures = failures_;
    result.repairs = repairs_;
    result.processed_events = processed_events_;
    result.final_state = static_cast<int>(repair_queue_.size());
    result.measured_time = std::accumulate(state_times_.begin(), state_times_.end(), 0.0);
    return result;
}
