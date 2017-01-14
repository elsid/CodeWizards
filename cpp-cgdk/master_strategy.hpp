#pragma once

#include "abstract_strategy.hpp"
#include "skills.hpp"

namespace strategy {

class MasterStrategy : public AbstractStrategy {
public:
    MasterStrategy(std::unique_ptr<AbstractStrategy> component, const Context& context);

    void apply(Context& context) override final;

private:
    const std::unique_ptr<AbstractStrategy> component_;
    const std::map<UnitId, Specialization> specializations_;

    void command(Context& context) const;
    void learn_skills(Context& context) const;
};

}
