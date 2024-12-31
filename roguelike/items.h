#pragma once

#include "entities.h"
//#include "objects.h"
#include "state.h"

struct Salve : public GameStateObject, IGameState::Item {
    explicit Salve(int heal);

    void apply(IGameState::Object *object) const override;

    int heal;
};

// Stick is basic weapon that just damages all enemies in the certain radius,
// it can be stick, sword, etc.
struct Stick : public GameStateObject, IGameState::Item {
    Stick();

    Stick(IGameState::ItemDescriptor item_descriptor, int damage, int radius);

    void apply(IGameState::Object *object) const override;

    int damage;
    int radius;
};