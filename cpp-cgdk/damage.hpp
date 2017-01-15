#pragma once

namespace strategy {

class Damage {
public:
    struct Physic {
        double value;
    };

    struct Magic {
        double value;
    };

    Damage() = default;
    Damage(Physic physic) : physic_(physic.value) {}
    Damage(Magic magic) : magic_(magic.value) {}
    Damage(Physic physic, Magic magic) : physic_(physic.value), magic_(magic.value) {}

    double physic() const {
        return physic_;
    }

    double magic() const {
        return magic_;
    }

    double sum() const {
        return physic() + magic();
    }

private:
    double physic_ = 0;
    double magic_ = 0;
};

inline Damage operator *(double factor, const Damage& damage) {
    return Damage(Damage::Physic {factor * damage.physic()}, Damage::Magic {factor * damage.magic()});
}

inline Damage operator *(const Damage& damage, double factor) {
    return factor * damage;
}

} // namespace strategy
