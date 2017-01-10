#include "master_strategy.hpp"

namespace strategy {

class MessageFactory {
public:
    MessageFactory& lane(model::LaneType value) {
        lane_ = value;
        return *this;
    }

    MessageFactory& skill_to_learn(model::SkillType value) {
        skill_to_learn_ = value;
        return *this;
    }

    MessageFactory& raw_message(std::vector<signed char> value) {
        raw_message_ = std::move(value);
        return *this;
    }

    model::Message product() const {
        return model::Message(lane_, skill_to_learn_, raw_message_);
    }

private:
    model::LaneType lane_ = model::_LANE_UNKNOWN_;
    model::SkillType skill_to_learn_ = model::_SKILL_UNKNOWN_;
    std::vector<signed char> raw_message_;
};

std::vector<model::Message> make_messages(const std::map<UnitId, MessageFactory>& factories) {
    std::vector<model::Message> result;
    result.reserve(factories.size());
    std::transform(factories.begin(), factories.end(), std::back_inserter(result),
            [&] (const auto& v) { return v.second.product(); });
    return result;
}

MasterStrategy::MasterStrategy(std::unique_ptr<Strategy> component, const Context &context)
        : component_(std::move(component)),
          specializations_(distribute_specializations(context)) {
}

void MasterStrategy::apply(Context &context) {
    command(context);
    component_->apply(context);
    learn_skills(context);
}

const std::array<model::LaneType, 4> LANES = {{
    model::LANE_TOP,
    model::LANE_MIDDLE,
    model::LANE_MIDDLE,
    model::LANE_BOTTOM,
}};

void MasterStrategy::command(Context& context) const {
    std::map<UnitId, MessageFactory> messages_factories;
    const auto& wizards = get_units<model::Wizard>(context.history_cache());

    for (const auto& v : specializations_) {
        if (v.first != context.self().getId()) {
            messages_factories[v.first].skill_to_learn(get_skill_to_recommend(wizards.at(v.first).value(), v.second));
        }
    }

    auto lane = LANES.begin();
    for (auto& v : messages_factories) {
        v.second.lane(*lane++);
    }

    const auto messages = make_messages(messages_factories);

    context.move().setMessages(messages);
}

void MasterStrategy::learn_skills(Context& context) const {
    if (context.self().getSkills().size() < std::size_t(context.self().getLevel())) {
        const auto specialization = specializations_.at(context.self().getId());
        const auto recommended = get_skill_to_recommend(context.self(), specialization);
        const auto skill = get_skill_to_learn(context, recommended);
        if (skill != model::_SKILL_UNKNOWN_) {
            context.move().setSkillToLearn(skill);
        }
    }
}

}
