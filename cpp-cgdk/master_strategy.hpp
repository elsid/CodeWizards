#pragma once

#include "base_strategy.hpp"
#include "skills.hpp"

namespace strategy {

class MasterStrategy : public Strategy {
public:
    MasterStrategy(std::unique_ptr<Strategy> component, const Context& context);

    void apply(Context& context) override final;

private:
    const std::unique_ptr<Strategy> component_;
    const std::map<UnitId, Specialization> specializations_;

    void command(Context& context) const;
    void learn_skills(Context& context) const;
};

}
