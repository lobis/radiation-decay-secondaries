
#include "PrimaryGeneratorAction.h"
#include "RunAction.h"
#include "DetectorConstruction.h"

#include <G4Event.hh>
#include <G4ParticleTable.hh>
#include <G4RunManager.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4IonTable.hh>

using namespace std;
using namespace CLHEP;

PrimaryGeneratorAction::PrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction() {
    gun.SetParticlePosition({0.0, 0.0, 0.0});
    gun.SetParticleMomentumDirection({0.0, 0.0, 1.0});
    gun.SetParticleEnergy(0.0);
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
    const double z = G4UniformRand() * DetectorConstruction::GetThickness();

    gun.SetParticlePosition({0.0, 0.0, z});

    if (!foundPrimaryParticle) {
        gun.SetParticleDefinition(FindPrimaryParticle());
        foundPrimaryParticle = true;
    }

    gun.GeneratePrimaryVertex(event);

    RunAction::IncreaseLaunchedPrimaries(gun.GetParticleDefinition()->GetParticleName());
}

G4ParticleDefinition *PrimaryGeneratorAction::FindPrimaryParticle() {

    string inputParticleName = RunAction::GetParticleName();
    for (int Z = 1; Z <= 110; Z++) {
        for (int A = 2 * Z - 1; A <= 3 * Z; A++) {
            auto particle = G4IonTable::GetIonTable()->GetIon(Z, A);
            if (!particle) {
                continue;
            }
            if (particle->GetParticleName() == inputParticleName) {
                return particle;
            }
        }
    }

    throw invalid_argument("Particle not found in the ion table");
}
