#pragma once

#include <point.hpp>
#include <optimal_movement.hpp>
#include <graph.hpp>

#include "model/World.h"
#include "model/Game.h"

#include <ostream>
#include <iomanip>
#include <limits>

namespace strategy {

template <class T>
inline std::ostream& write(std::ostream& stream, const std::vector<T>& values) {
    stream << "{";
    for (const auto& v : values) {
        stream << v << ", ";
    }
    return stream << "}";
}

inline std::ostream& operator <<(std::ostream& stream, const std::vector<int>& values) {
    return write(stream, values);
}

inline std::ostream& operator <<(std::ostream& stream, const std::vector<Point>& values) {
    return write(stream, values);
}

inline std::ostream& operator <<(std::ostream& stream, const std::vector<Graph::Node>& values) {
    return write(stream, values);
}

template <class T>
inline std::ostream& operator <<(std::ostream& stream, const std::vector<T>&) {
    stream << "{";
//    for (const auto& v : values) {
//        stream << v << ", ";
//    }
    return stream << "}";
}

inline std::ostream& operator <<(std::ostream& stream, const Point& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "Point(" << value.x() << ", " << value.y() << ")";
}

inline std::ostream&operator <<(std::ostream& stream, const MovementState& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "MovementState(" << value.tick() << ", " << value.position() << ", " << value.angle() << ")";
}

inline std::ostream&operator <<(std::ostream& stream, const Movement& value) {
    return stream << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "Movement(" << value.speed() << ", " << value.strafe_speed() << ", " << value.turn() << ")";
}

inline std::ostream& operator <<(std::ostream& stream, model::Faction value) {
    switch (value) {
        case model::_FACTION_UNKNOWN_:
            return stream << "model::_FACTION_UNKNOWN_";
        case model::FACTION_ACADEMY:
            return stream << "model::FACTION_ACADEMY";
        case model::FACTION_RENEGADES:
            return stream << "model::FACTION_RENEGADES";
        case model::FACTION_NEUTRAL:
            return stream << "model::FACTION_NEUTRAL";
        case model::FACTION_OTHER:
            return stream << "model::FACTION_OTHER";
        case model::_FACTION_COUNT_:
            return stream << "model::_FACTION_COUNT_";
    }
    throw std::logic_error("Invalid model::Faction value: " + std::to_string(value));
}

inline std::ostream& operator <<(std::ostream& stream, const model::World& value) {
    stream << "model::World(\n";
    stream << "    " << value.getTickIndex() << ", // TickIndex\n";
    stream << "    " << value.getTickCount() << ", // TickCount\n";
    stream << "    " << value.getWidth() << ", // Width\n";
    stream << "    " << value.getHeight() << ", // Height\n";
    stream << "    " << value.getPlayers() << ", // Players\n";
    stream << "    " << value.getWizards() << ", // Wizards\n";
    stream << "    " << value.getMinions() << ", // Minions\n";
    stream << "    " << value.getProjectiles() << ", // Projectiles\n";
    stream << "    " << value.getBonuses() << ", // Bonuses\n";
    stream << "    " << value.getBuildings() << ", // Buildings\n";
    stream << "    " << value.getTrees() << " // Trees\n";
    return stream << ")";
}

inline std::ostream& operator <<(std::ostream& stream, const model::Tree& value) {
    stream << "model::Tree(\n";
    stream << "    " << value.getId() << ", // Id\n";
    stream << "    " << value.getX() << ", // X\n";
    stream << "    " << value.getY() << ", // Y\n";
    stream << "    " << value.getSpeedX() << ", // SpeedX\n";
    stream << "    " << value.getSpeedY() << ", // SpeedY\n";
    stream << "    " << value.getAngle() << ", // Angle\n";
    stream << "    " << value.getFaction() << ", // Faction\n";
    stream << "    " << value.getRadius() << ", // Radius\n";
    stream << "    " << value.getLife() << ", // Life\n";
    stream << "    " << value.getMaxLife() << ", // MaxLife\n";
    stream << "    " << value.getStatuses() << " // Statuses\n";
    return stream << ")";
}

inline std::ostream& operator <<(std::ostream& stream, const model::Wizard& value) {
    stream << "model::Wizard(\n";
    stream << "    " << value.getId() << ", // Id\n";
    stream << "    " << value.getX() << ", // X\n";
    stream << "    " << value.getY() << ", // Y\n";
    stream << "    " << value.getSpeedX() << ", // SpeedX\n";
    stream << "    " << value.getSpeedY() << ", // SpeedY\n";
    stream << "    " << value.getAngle() << ", // Angle\n";
    stream << "    " << value.getFaction() << ", // Faction\n";
    stream << "    " << value.getRadius() << ", // Radius\n";
    stream << "    " << value.getLife() << ", // Life\n";
    stream << "    " << value.getMaxLife() << ", // MaxLife\n";
    stream << "    " << value.getStatuses() << ", // Statuses\n";
    stream << "    " << value.getOwnerPlayerId() << ", // OwnerPlayerId\n";
    stream << "    " << value.isMe() << ", // Me\n";
    stream << "    " << value.getMana() << ", // Mana\n";
    stream << "    " << value.getMaxMana() << ", // MaxMana\n";
    stream << "    " << value.getVisionRange() << ", // VisionRange\n";
    stream << "    " << value.getCastRange() << ", // CastRange\n";
    stream << "    " << value.getXp() << ", // Xp\n";
    stream << "    " << value.getLevel() << ", // Level\n";
    stream << "    " << value.getSkills() << ", // Skills\n";
    stream << "    " << value.getRemainingActionCooldownTicks() << ", // RemainingActionCooldownTicks\n";
    stream << "    " << value.getRemainingCooldownTicksByAction() << ", // RemainingCooldownTicksByAction\n";
    stream << "    " << value.isMaster() << ", // Master\n";
    stream << "    " << value.getMessages() << " // Messages\n";
    return stream << ")";
}

inline std::ostream& operator <<(std::ostream& stream, const model::Game& value) {
    stream << "model::Game(\n";
    stream << "    " << value.getRandomSeed() << ", // RandomSeed\n";
    stream << "    " << value.getTickCount() << ", // TickCount\n";
    stream << "    " << value.getMapSize() << ", // MapSize\n";
    stream << "    " << value.isSkillsEnabled() << ", // SkillsEnabled\n";
    stream << "    " << value.isRawMessagesEnabled() << ", // RawMessagesEnabled\n";
    stream << "    " << value.getFriendlyFireDamageFactor() << ", // FriendlyFireDamageFactor\n";
    stream << "    " << value.getBuildingDamageScoreFactor() << ", // BuildingDamageScoreFactor\n";
    stream << "    " << value.getBuildingEliminationScoreFactor() << ", // BuildingEliminationScoreFactor\n";
    stream << "    " << value.getMinionDamageScoreFactor() << ", // MinionDamageScoreFactor\n";
    stream << "    " << value.getMinionEliminationScoreFactor() << ", // MinionEliminationScoreFactor\n";
    stream << "    " << value.getWizardDamageScoreFactor() << ", // WizardDamageScoreFactor\n";
    stream << "    " << value.getWizardEliminationScoreFactor() << ", // WizardEliminationScoreFactor\n";
    stream << "    " << value.getTeamWorkingScoreFactor() << ", // TeamWorkingScoreFactor\n";
    stream << "    " << value.getVictoryScore() << ", // VictoryScore\n";
    stream << "    " << value.getScoreGainRange() << ", // ScoreGainRange\n";
    stream << "    " << value.getRawMessageMaxLength() << ", // RawMessageMaxLength\n";
    stream << "    " << value.getRawMessageTransmissionSpeed() << ", // RawMessageTransmissionSpeed\n";
    stream << "    " << value.getWizardRadius() << ", // WizardRadius\n";
    stream << "    " << value.getWizardCastRange() << ", // WizardCastRange\n";
    stream << "    " << value.getWizardVisionRange() << ", // WizardVisionRange\n";
    stream << "    " << value.getWizardForwardSpeed() << ", // WizardForwardSpeed\n";
    stream << "    " << value.getWizardBackwardSpeed() << ", // WizardBackwardSpeed\n";
    stream << "    " << value.getWizardStrafeSpeed() << ", // WizardStrafeSpeed\n";
    stream << "    " << value.getWizardBaseLife() << ", // WizardBaseLife\n";
    stream << "    " << value.getWizardLifeGrowthPerLevel() << ", // WizardLifeGrowthPerLevel\n";
    stream << "    " << value.getWizardBaseMana() << ", // WizardBaseMana\n";
    stream << "    " << value.getWizardManaGrowthPerLevel() << ", // WizardManaGrowthPerLevel\n";
    stream << "    " << value.getWizardBaseLifeRegeneration() << ", // WizardBaseLifeRegeneration\n";
    stream << "    " << value.getWizardLifeRegenerationGrowthPerLevel() << ", // WizardLifeRegenerationGrowthPerLevel\n";
    stream << "    " << value.getWizardBaseManaRegeneration() << ", // WizardBaseManaRegeneration\n";
    stream << "    " << value.getWizardManaRegenerationGrowthPerLevel() << ", // WizardManaRegenerationGrowthPerLevel\n";
    stream << "    " << value.getWizardMaxTurnAngle() << ", // WizardMaxTurnAngle\n";
    stream << "    " << value.getWizardMaxResurrectionDelayTicks() << ", // WizardMaxResurrectionDelayTicks\n";
    stream << "    " << value.getWizardMinResurrectionDelayTicks() << ", // WizardMinResurrectionDelayTicks\n";
    stream << "    " << value.getWizardActionCooldownTicks() << ", // WizardActionCooldownTicks\n";
    stream << "    " << value.getStaffCooldownTicks() << ", // StaffCooldownTicks\n";
    stream << "    " << value.getMagicMissileCooldownTicks() << ", // MagicMissileCooldownTicks\n";
    stream << "    " << value.getFrostBoltCooldownTicks() << ", // FrostBoltCooldownTicks\n";
    stream << "    " << value.getFireballCooldownTicks() << ", // FireballCooldownTicks\n";
    stream << "    " << value.getHasteCooldownTicks() << ", // HasteCooldownTicks\n";
    stream << "    " << value.getShieldCooldownTicks() << ", // ShieldCooldownTicks\n";
    stream << "    " << value.getMagicMissileManacost() << ", // MagicMissileManacost\n";
    stream << "    " << value.getFrostBoltManacost() << ", // FrostBoltManacost\n";
    stream << "    " << value.getFireballManacost() << ", // FireballManacost\n";
    stream << "    " << value.getHasteManacost() << ", // HasteManacost\n";
    stream << "    " << value.getShieldManacost() << ", // ShieldManacost\n";
    stream << "    " << value.getStaffDamage() << ", // StaffDamage\n";
    stream << "    " << value.getStaffSector() << ", // StaffSector\n";
    stream << "    " << value.getStaffRange() << ", // StaffRange\n";
    stream << "    " << value.getLevelUpXpValues() << ", // LevelUpXpValues\n";
    stream << "    " << value.getMinionRadius() << ", // MinionRadius\n";
    stream << "    " << value.getMinionVisionRange() << ", // MinionVisionRange\n";
    stream << "    " << value.getMinionSpeed() << ", // MinionSpeed\n";
    stream << "    " << value.getMinionMaxTurnAngle() << ", // MinionMaxTurnAngle\n";
    stream << "    " << value.getMinionLife() << ", // MinionLife\n";
    stream << "    " << value.getFactionMinionAppearanceIntervalTicks() << ", // FactionMinionAppearanceIntervalTicks\n";
    stream << "    " << value.getOrcWoodcutterActionCooldownTicks() << ", // OrcWoodcutterActionCooldownTicks\n";
    stream << "    " << value.getOrcWoodcutterDamage() << ", // OrcWoodcutterDamage\n";
    stream << "    " << value.getOrcWoodcutterAttackSector() << ", // OrcWoodcutterAttackSector\n";
    stream << "    " << value.getOrcWoodcutterAttackRange() << ", // OrcWoodcutterAttackRange\n";
    stream << "    " << value.getFetishBlowdartActionCooldownTicks() << ", // FetishBlowdartActionCooldownTicks\n";
    stream << "    " << value.getFetishBlowdartAttackRange() << ", // FetishBlowdartAttackRange\n";
    stream << "    " << value.getFetishBlowdartAttackSector() << ", // FetishBlowdartAttackSector\n";
    stream << "    " << value.getBonusRadius() << ", // BonusRadius\n";
    stream << "    " << value.getBonusAppearanceIntervalTicks() << ", // BonusAppearanceIntervalTicks\n";
    stream << "    " << value.getBonusScoreAmount() << ", // BonusScoreAmount\n";
    stream << "    " << value.getDartRadius() << ", // DartRadius\n";
    stream << "    " << value.getDartSpeed() << ", // DartSpeed\n";
    stream << "    " << value.getDartDirectDamage() << ", // DartDirectDamage\n";
    stream << "    " << value.getMagicMissileRadius() << ", // MagicMissileRadius\n";
    stream << "    " << value.getMagicMissileSpeed() << ", // MagicMissileSpeed\n";
    stream << "    " << value.getMagicMissileDirectDamage() << ", // MagicMissileDirectDamage\n";
    stream << "    " << value.getFrostBoltRadius() << ", // FrostBoltRadius\n";
    stream << "    " << value.getFrostBoltSpeed() << ", // FrostBoltSpeed\n";
    stream << "    " << value.getFrostBoltDirectDamage() << ", // FrostBoltDirectDamage\n";
    stream << "    " << value.getFireballRadius() << ", // FireballRadius\n";
    stream << "    " << value.getFireballSpeed() << ", // FireballSpeed\n";
    stream << "    " << value.getFireballExplosionMaxDamageRange() << ", // FireballExplosionMaxDamageRange\n";
    stream << "    " << value.getFireballExplosionMinDamageRange() << ", // FireballExplosionMinDamageRange\n";
    stream << "    " << value.getFireballExplosionMaxDamage() << ", // FireballExplosionMaxDamage\n";
    stream << "    " << value.getFireballExplosionMinDamage() << ", // FireballExplosionMinDamage\n";
    stream << "    " << value.getGuardianTowerRadius() << ", // GuardianTowerRadius\n";
    stream << "    " << value.getGuardianTowerVisionRange() << ", // GuardianTowerVisionRange\n";
    stream << "    " << value.getGuardianTowerLife() << ", // GuardianTowerLife\n";
    stream << "    " << value.getGuardianTowerAttackRange() << ", // GuardianTowerAttackRange\n";
    stream << "    " << value.getGuardianTowerDamage() << ", // GuardianTowerDamage\n";
    stream << "    " << value.getGuardianTowerCooldownTicks() << ", // GuardianTowerCooldownTicks\n";
    stream << "    " << value.getFactionBaseRadius() << ", // FactionBaseRadius\n";
    stream << "    " << value.getFactionBaseVisionRange() << ", // FactionBaseVisionRange\n";
    stream << "    " << value.getFactionBaseLife() << ", // FactionBaseLife\n";
    stream << "    " << value.getFactionBaseAttackRange() << ", // FactionBaseAttackRange\n";
    stream << "    " << value.getFactionBaseDamage() << ", // FactionBaseDamage\n";
    stream << "    " << value.getFactionBaseCooldownTicks() << ", // FactionBaseCooldownTicks\n";
    stream << "    " << value.getBurningDurationTicks() << ", // BurningDurationTicks\n";
    stream << "    " << value.getBurningSummaryDamage() << ", // BurningSummaryDamage\n";
    stream << "    " << value.getEmpoweredDurationTicks() << ", // EmpoweredDurationTicks\n";
    stream << "    " << value.getEmpoweredDamageFactor() << ", // EmpoweredDamageFactor\n";
    stream << "    " << value.getFrozenDurationTicks() << ", // FrozenDurationTicks\n";
    stream << "    " << value.getHastenedDurationTicks() << ", // HastenedDurationTicks\n";
    stream << "    " << value.getHastenedBonusDurationFactor() << ", // HastenedBonusDurationFactor\n";
    stream << "    " << value.getHastenedMovementBonusFactor() << ", // HastenedMovementBonusFactor\n";
    stream << "    " << value.getHastenedRotationBonusFactor() << ", // HastenedRotationBonusFactor\n";
    stream << "    " << value.getShieldedDurationTicks() << ", // ShieldedDurationTicks\n";
    stream << "    " << value.getShieldedBonusDurationFactor() << ", // ShieldedBonusDurationFactor\n";
    stream << "    " << value.getShieldedDirectDamageAbsorptionFactor() << ", // ShieldedDirectDamageAbsorptionFactor\n";
    stream << "    " << value.getAuraSkillRange() << ", // AuraSkillRange\n";
    stream << "    " << value.getRangeBonusPerSkillLevel() << ", // RangeBonusPerSkillLevel\n";
    stream << "    " << value.getMagicalDamageBonusPerSkillLevel() << ", // MagicalDamageBonusPerSkillLevel\n";
    stream << "    " << value.getStaffDamageBonusPerSkillLevel() << ", // StaffDamageBonusPerSkillLevel\n";
    stream << "    " << value.getMovementBonusFactorPerSkillLevel() << ", // MovementBonusFactorPerSkillLevel\n";
    stream << "    " << value.getMagicalDamageAbsorptionPerSkillLevel() << " // MagicalDamageAbsorptionPerSkillLevel\n";
    return stream << ")";
}

} // namespace strategy
