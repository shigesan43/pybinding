#pragma once
#include "support/dense.hpp"
#include "utils/Chrono.hpp"
#include "utils/Log.hpp"
#include <memory>

namespace tbm {

class Model;
class Hamiltonian;
template <typename scalar_t> class HamiltonianT;

/**
 Abstract base class for Green's function strategy.
 */
class GreensStrategy {
public:
    virtual ~GreensStrategy() = default;

    /// Try to set the Hamiltonian
    /// @return false if the given Hamiltonian is the wrong scalar type for this GreensStrategy
    virtual bool set_hamiltonian(const std::shared_ptr<const Hamiltonian>& hamiltonian) = 0;
    /// Return the Green's function at (i,j) for the given energy range
    virtual ArrayXcf calculate(int i, int j, ArrayXd energy, float broadening) = 0;
    /// Get some information about what happened during the last calculation
    virtual std::string report(bool shortform) const = 0;
};


/**
 Abstract base with type specialization.
 */
template<typename scalar_t>
class GreensStrategyT : public GreensStrategy {
public:
    virtual ~GreensStrategyT() { Log::d("~GreensStrategy<" + num::scalar_name<scalar_t>() + ">()"); }

    virtual bool set_hamiltonian(const std::shared_ptr<const Hamiltonian>& ham) final {
        // check if it's compatible
        if (auto cast_ham = std::dynamic_pointer_cast<const HamiltonianT<scalar_t>>(ham)) {
            if (hamiltonian != cast_ham) {
                hamiltonian = cast_ham;
                hamiltonian_changed();
            }
            return true;
        }
        // failed -> wrong scalar_type
        return false;
    }

protected:
    /// post-processing that may be defined by derived classes
    virtual void hamiltonian_changed() {};

protected:
    std::shared_ptr<const HamiltonianT<scalar_t>> hamiltonian; ///< the Hamiltonian to solve
};


/**
 Green's function calculation interface.
 Internally it uses a GreensStrategy with the scalar of the given Hamiltonian.
 Derived classes must implement create_strategy_for(hamiltonian).
 */
class Greens {
public:
    virtual ~Greens() = default;

public:
    void set_model(const std::shared_ptr<const Model>& new_model);

    /// Get some information about what happened during the last calculation
    std::string report(bool shortform) const {
        return strategy ? strategy->report(shortform) + " " + calculation_timer.str() : "";
    }

protected:
    /// Create a new Green's strategy object for the given Hamiltonian
    virtual std::unique_ptr<GreensStrategy>
        create_strategy_for(const std::shared_ptr<const Hamiltonian>&) const = 0;

protected:
    std::shared_ptr<const Model> model;
    std::unique_ptr<GreensStrategy> strategy;
    Chrono calculation_timer; ///< last calculation time
};

} // namespace tbm
