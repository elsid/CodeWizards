#include "stats.hpp"
#include "base_strategy.hpp"

#ifdef ELSID_STRATEGY_LOCAL
#include <iostream>
#endif

namespace strategy {

Stats::Stats(const BaseStrategy& base_strategy)
        : base_strategy_(base_strategy) {
}

#ifdef ELSID_STRATEGY_LOCAL
Stats::~Stats() {
    log(std::cout);
}
#endif

void Stats::calculate(const Context& context) {
    last_tick_ = context.world().getTickIndex();
    last_life_ = context.self().getLife();

    if (base_strategy_.target().is<model::Building>()) {
        ++buildings_.target_ticks_count;
    } else if (base_strategy_.target().is<model::Minion>()) {
        ++minions_.target_ticks_count;
    } else if (base_strategy_.target().is<model::Tree>()) {
        ++trees_.target_ticks_count;
    } else if (base_strategy_.target().is<model::Wizard>()) {
        ++wizards_.target_ticks_count;
    }

    for (const auto& projectile : get_units<model::Projectile>(context.history_cache())) {
        if (projectile.second.last_seen() == context.world().getTickIndex() - 1
                && projectile.second.value().getOwnerUnitId() == context.self().getId()) {
            ++casts_count_;

            const auto is_hit = [&] (const auto& unit) {
                return unit.getFaction() != context.self().getFaction()
                        && (get_position(projectile.second.value()) + get_speed(projectile.second.value())).distance(get_position(unit))
                            <= unit.getRadius() + projectile.second.value().getRadius() + get_speed(unit).norm();
            };

            const auto hits_count = [&] (const auto& units) {
                return std::count_if(units.begin(), units.end(), [&] (const auto& v) { return is_hit(v.second.value()); });
            };

            const auto buildings_hits_count = hits_count(get_units<model::Building>(context.history_cache()));
            const auto minions_hits_count = hits_count(get_units<model::Minion>(context.history_cache()));
            const auto trees_hits_count = hits_count(get_units<model::Tree>(context.history_cache()));
            const auto wizards_hits_count = hits_count(get_units<model::Wizard>(context.history_cache()));

            const auto units_hits_count = buildings_hits_count + minions_hits_count + trees_hits_count + wizards_hits_count;

            if (const auto target = base_strategy_.target().unit<model::Building>(context.history_cache())) {
                buildings_.target_hits_count += is_hit(*target);
                ++buildings_.target_casts_count;
            } else if (const auto target = base_strategy_.target().unit<model::Minion>(context.history_cache())) {
                minions_.target_hits_count += is_hit(*target);
                ++minions_.target_casts_count;
            } else if (const auto target = base_strategy_.target().unit<model::Tree>(context.history_cache())) {
                trees_.target_hits_count += is_hit(*target);
                ++trees_.target_casts_count;
            } else if (const auto target = base_strategy_.target().unit<model::Wizard>(context.history_cache())) {
                wizards_.target_hits_count += is_hit(*target);
                ++wizards_.target_casts_count;
            }

            units_hits_count_ += units_hits_count;

            hits_count_ += bool(units_hits_count);
            buildings_.hits_count += bool(buildings_hits_count);
            minions_.hits_count += bool(minions_hits_count);
            trees_.hits_count += bool(trees_hits_count);
            wizards_.hits_count += bool(wizards_hits_count);

            target_ticks_count_ = buildings_.target_ticks_count + minions_.target_ticks_count + trees_.target_ticks_count + wizards_.target_ticks_count;

            hits_per_casts_ = double(hits_count_) / double(casts_count_ ? casts_count_ : 1);
            target_casts_per_target_ticks_ = double(casts_count_) / double(target_ticks_count_ ? target_ticks_count_ : 1);
            target_casts_per_ticks_ = double(target_ticks_count_) / double(last_tick_ + 1);

            fill(buildings_);
            fill(minions_);
            fill(trees_);
            fill(wizards_);

            log_hits_per_casts(SLOG(context));
            log_target_casts_per_target_ticks(SLOG(context));
            log_target_casts_per_ticks(SLOG(context));
        }
    }

    if (prev_my_life_ < context.self().getLife()) {
        prev_my_life_ = context.self().getLife();
    } else if (prev_my_life_ > context.self().getLife()) {
        last_damage_ = prev_my_life_ - context.self().getLife();
        sum_damage_to_me_ += last_damage_;
        prev_my_life_ = context.self().getLife();
        log_damage_to_me(SLOG(context));
    }

    if (prev_tick_ && last_tick_ - prev_tick_ >= context.game().getWizardMinResurrectionDelayTicks()) {
        ++deaths_count_;
        log_deaths_count(SLOG(context));
    }

    prev_tick_ = context.world().getTickIndex();
}

void Stats::fill(UnitsStats& stats) const {
    stats.target_hits_per_target_casts_ = double(stats.target_hits_count)
        / double(stats.target_casts_count ? stats.target_casts_count : 1);
    stats.hits_per_casts_ = double(stats.hits_count) / double(casts_count_ ? casts_count_ : 1);
    stats.target_casts_per_target_ticks_ = double(stats.target_casts_count) / double(stats.target_ticks_count ? stats.target_ticks_count : 1);
}

template <class Stream>
Stream& Stats::log(Stream& stream) const {
    log_hits_per_casts(stream);
    log_target_casts_per_target_ticks(stream);
    log_target_casts_per_ticks(stream);
    log_damage_to_me(stream);
    log_deaths_count(stream);
    return stream;
}

struct HitsPerCasts {
    const Stats::UnitsStats& stats;
};

std::ostream& operator <<(std::ostream& stream, const HitsPerCasts& value) {
    return stream
        << " t: "
        << value.stats.target_hits_count << "/"
        << value.stats.target_casts_count << "="
        << value.stats.target_hits_per_target_casts_
        << " o: "
        << value.stats.hits_count << "/*="
        << value.stats.hits_per_casts_;
}

template <class Stream>
Stream& Stats::log_hits_per_casts(Stream& stream) const {
    return stream << "hits_per_casts"
        << " all: " << hits_count_ << "/" << casts_count_ << "=" << hits_per_casts_ << "; "
        << "buildings:" << HitsPerCasts {buildings_} << "; "
        << "minions:" << HitsPerCasts {minions_} << "; "
        << "trees:" << HitsPerCasts {trees_} << "; "
        << "wizards:" << HitsPerCasts {wizards_}
        << '\n';
}

struct TargetCastsPerTargetTicks {
    const Stats::UnitsStats& stats;
};

std::ostream& operator <<(std::ostream& stream, const TargetCastsPerTargetTicks& value) {
    return stream
        << value.stats.target_casts_count << "/"
        << value.stats.target_ticks_count << "="
        << value.stats.target_casts_per_target_ticks_ << "~"
        << value.stats.target_casts_per_target_ticks_ * 60;
}

template <class Stream>
Stream& Stats::log_target_casts_per_target_ticks(Stream& stream) const {
    return stream << "target_casts_per_target_ticks"
        << " all: "
        << casts_count_ << "/"
        << target_ticks_count_ << "="
        << target_casts_per_target_ticks_ << "~"
        << target_casts_per_target_ticks_ * 60 << " "
        << "buildings: " << TargetCastsPerTargetTicks {buildings_} << "; "
        << "minions: " << TargetCastsPerTargetTicks {minions_} << "; "
        << "trees: " << TargetCastsPerTargetTicks {trees_} << "; "
        << "wizards: " << TargetCastsPerTargetTicks {wizards_}
        << '\n';
}

template <class Stream>
Stream& Stats::log_target_casts_per_ticks(Stream& stream) const {
    return stream << "target_ticks_per_ticks"
        << " all: "
        << target_ticks_count_ << "/"
        << last_tick_ << "="
        << target_casts_per_ticks_ << " "
        << '\n';
}

template <class Stream>
Stream& Stats::log_damage_to_me(Stream& stream) const {
    return stream << "damage_to_me"
        << " value: " << last_damage_
        << " life: " << last_life_
        << " sum: " << sum_damage_to_me_
        << '\n';
}

template <class Stream>
Stream& Stats::log_deaths_count(Stream& stream) const {
    return stream << "death count: " << deaths_count_ << " tick: " << prev_tick_ + 1 << '\n';
}

}
