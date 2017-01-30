#pragma once

#include "cache.hpp"

#include <sstream>

namespace strategy {

template <class T>
typename Cache<T>::Units::const_iterator find_unit(const FullCache& cache, Id<T> id) {
    const auto& units = get_units<T>(cache);
    return std::find_if(units.begin(), units.end(), [&] (const auto& unit) { return unit.first == id.value(); });
}

template <class T>
bool is_end(const FullCache& cache, const typename Cache<T>::Units::const_iterator it) {
    return get_units<T>(cache).end() == it;
}

template <class T>
Id<T> get_id(const T& unit) {
    return Id<T>(unit.getId());
}

class Target {
    friend bool operator !=(const Target& lhs, const Target& rhs);

public:
    template <class T>
    using Pair = std::pair<bool, Id<T>>;

    Target() = default;

    template <class T>
    Target(Id<T> id) {
        std::get<Pair<T>>(ids_) = {true, id};
    }

    template <class T>
    Id<T> id() const {
        return std::get<Pair<T>>(ids_).second;
    }

    template <class T>
    bool is() const {
        return std::get<Pair<T>>(ids_).first;
    }

    bool is_some() const {
        return is<model::Bonus>() || is<model::Building>() || is<model::Minion>() || is<model::Wizard>() || is<model::Tree>();
    }

    template <class T>
    const CachedUnit<T>* cached_unit(const FullCache& cache) const {
        if (!is<T>()) {
            return nullptr;
        }
        const auto it = find_unit(cache, id<T>());
        return is_end<T>(cache, it) ? nullptr : &it->second;
    }

    template <class T>
    const T* unit(const FullCache& cache) const {
        const auto cached = cached_unit<T>(cache);
        return cached ? &cached->value() : nullptr;
    }

    const model::CircularUnit* circular_unit(const FullCache& cache) const {
        if (is<model::Bonus>()) {
            return unit<model::Bonus>(cache);
        } else if (is<model::Building>()) {
            return unit<model::Building>(cache);
        } else if (is<model::Minion>()) {
            return unit<model::Minion>(cache);
        } else if (is<model::Wizard>()) {
            return unit<model::Wizard>(cache);
        } else if (is<model::Tree>()) {
            return unit<model::Tree>(cache);
        }
        return nullptr;
    }

    template <class Function>
    auto apply(const FullCache& cache, Function function) const {
        if (is<model::Bonus>()) {
            return function(unit<model::Bonus>(cache));
        } else if (is<model::Building>()) {
            return function(unit<model::Building>(cache));
        } else if (is<model::Minion>()) {
            return function(unit<model::Minion>(cache));
        } else if (is<model::Wizard>()) {
            return function(unit<model::Wizard>(cache));
        } else if (is<model::Tree>()) {
            return function(unit<model::Tree>(cache));
        }
        std::ostringstream error;
        error << "Target is not set in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
        throw std::logic_error(error.str());
    }

    template <class Function>
    auto apply_cached(const FullCache& cache, Function function) const {
        if (is<model::Bonus>()) {
            return function(cached_unit<model::Bonus>(cache));
        } else if (is<model::Building>()) {
            return function(cached_unit<model::Building>(cache));
        } else if (is<model::Minion>()) {
            return function(cached_unit<model::Minion>(cache));
        } else if (is<model::Wizard>()) {
            return function(cached_unit<model::Wizard>(cache));
        } else if (is<model::Tree>()) {
            return function(cached_unit<model::Tree>(cache));
        }
        std::ostringstream error;
        error << "Target is not set in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__;
        throw std::logic_error(error.str());
    }

private:
    std::tuple<
        std::pair<bool, Id<model::Bonus>>,
        std::pair<bool, Id<model::Building>>,
        std::pair<bool, Id<model::Minion>>,
        std::pair<bool, Id<model::Wizard>>,
        std::pair<bool, Id<model::Tree>>
    > ids_;
};

inline bool operator !=(const Target& lhs, const Target& rhs) {
    return lhs.ids_ != rhs.ids_;
}

}
