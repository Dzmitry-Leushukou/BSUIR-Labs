#include "repair_model.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>

namespace {
void check(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void near(double actual, double expected, double tolerance, const std::string& name) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= tolerance,
          name + ": actual=" + std::to_string(actual) + ", expected=" + std::to_string(expected));
}

SimulationResult run(const ModelParameters& model, const SimulationParameters& parameters) {
    RepairSimulation simulation(model, parameters);
    while (!simulation.advance(10000)) {}
    near(simulation.progress(), 1.0, 0.0, "finished progress");
    return simulation.result();
}

void check_accounting(const SimulationResult& result, double duration) {
    const auto& estimates = result.estimates;
    near(result.measured_time, duration, duration * 1e-10, "observation window");
    near(std::accumulate(estimates.probabilities.begin(), estimates.probabilities.end(), 0.0),
         1.0, 1e-10, "probability normalization");
    near(estimates.average_faulty, estimates.average_waiting + estimates.busy_probability,
         1e-10, "queue + operator = faulty");
    check(static_cast<std::int64_t>(result.initial_state) + static_cast<std::int64_t>(result.failures)
          - static_cast<std::int64_t>(result.repairs) == result.final_state, "failure/repair event balance");
    check(result.initial_state == 0, "observation must start with all machines working");
    check(result.processed_events == result.failures + result.repairs, "all events must be measured");
    near(estimates.average_repaired, static_cast<double>(result.repairs) / duration, 0.0,
         "throughput from actual completions");
    for (double probability : estimates.probabilities) {
        check(probability >= 0.0 && probability <= 1.0 + 1e-10, "probability range");
    }
}

double distribution_error(const ModelResult& theory, const ModelResult& empirical) {
    double error = 0.0;
    for (std::size_t k = 0; k < theory.probabilities.size(); ++k) {
        error += std::abs(theory.probabilities[k] - empirical.probabilities[k]);
    }
    return error;
}
}

int main() {
    try {
        // Closed-form two-state reference, not the model recurrence.
        const auto single = calculate_model({1, 0.2, 1.0});
        near(single.probabilities[1], 0.2 / 1.2, 1e-14, "single-machine busy");
        near(single.average_waiting, 0.0, 0.0, "single-machine queue");
        const auto theory = calculate_model({8, 0.2, 1.0});
        near(theory.probabilities[0], 0.070047852209567, 1e-13, "variant 17 p0");
        near(theory.average_faulty, 3.3502392610478355, 1e-12, "variant 17 faulty");
        near(theory.average_waiting, 2.420287113257402, 1e-12, "variant 17 queue");

        const ModelParameters cases[] = {{1, 0.2, 1.0}, {8, 0.2, 1.0}, {12, 0.01, 0.5}, {5, 3.0, 2.0}};
        for (const auto& model : cases) {
            const auto expected = calculate_model(model);
            const auto observed = run(model, {500000.0, 2026});
            check_accounting(observed, 500000.0);
            for (std::size_t k = 0; k < expected.probabilities.size(); ++k) {
                near(observed.estimates.probabilities[k], expected.probabilities[k], 0.012,
                     "long-run state " + std::to_string(k));
            }
            near(observed.estimates.average_faulty, expected.average_faulty, 0.06, "long-run faulty");
            near(observed.estimates.average_waiting, expected.average_waiting, 0.06, "long-run queue");
            near(observed.estimates.busy_probability, expected.busy_probability, 0.012, "long-run busy");
            near(observed.estimates.average_repaired, expected.average_repaired, 0.012, "long-run throughput");
        }

        // No events before a tiny horizon: include the whole final censored interval.
        const auto quiet = run({1, 1e-12, 1.0}, {0.25, 42});
        check_accounting(quiet, 0.25);
        near(quiet.estimates.probabilities[0], 1.0, 0.0, "no events in horizon");
        check(quiet.repairs == 0 && quiet.failures == 0, "events after horizon must not be counted");

        // Replaying a seed and changing GUI chunk sizes must not change the experiment.
        const auto replay = run({8, 0.2, 1.0}, {10000.0, 42});
        RepairSimulation chunked({8, 0.2, 1.0}, {10000.0, 42});
        while (!chunked.advance(1)) {}
        const auto one_by_one = chunked.result();
        check(one_by_one.estimates.probabilities == replay.estimates.probabilities &&
              one_by_one.repairs == replay.repairs, "seed replay and chunk independence");

        const auto short_run = run({8, 0.2, 1.0}, {1000.0, 42});
        check_accounting(short_run, 1000.0);
        check(std::abs(short_run.estimates.average_repaired -
                       short_run.estimates.mu * short_run.estimates.busy_probability) > 1e-6,
              "experimental A must not reuse the theoretical rate formula");

        double short_error = 0.0;
        double long_error = 0.0;
        for (std::uint64_t seed = 1; seed <= 5; ++seed) {
            short_error += distribution_error(theory, run({8, 0.2, 1.0}, {100.0, seed}).estimates);
            long_error += distribution_error(theory, run({8, 0.2, 1.0}, {100000.0, seed}).estimates);
        }
        check(long_error < short_error * 0.25, "longer observation should improve aggregate accuracy");
        std::cout << "All checks passed. Mean L1 probability error: T=100: " << short_error / 5.0
                  << "; T=100000: " << long_error / 5.0 << '\n';

        const auto demo = run({8, 0.2, 1.0}, {100000.0, 42});
        std::cout << "Default experiment (seed=42): A=" << demo.estimates.average_repaired
                  << ", w=" << demo.estimates.average_faulty << ", R=" << demo.estimates.average_waiting
                  << ", Pbusy=" << demo.estimates.busy_probability << ", repairs=" << demo.repairs << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
