
#include "TrackingAction.h"
#include "RunAction.h"

#include <G4ParticleDefinition.hh>
#include <G4SystemOfUnits.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>
#include <iostream>

#include <G4NeutrinoE.hh>
#include <G4NeutrinoMu.hh>
#include <G4NeutrinoTau.hh>
#include <G4AntiNeutrinoE.hh>
#include <G4AntiNeutrinoMu.hh>
#include <G4AntiNeutrinoTau.hh>

using namespace std;
using namespace CLHEP;

TrackingAction::TrackingAction() : G4UserTrackingAction() {
    fParticlesToIgnore = {
            G4NeutrinoE::Definition(),
            G4NeutrinoMu::Definition(),
            G4NeutrinoTau::Definition(),
            G4AntiNeutrinoE::Definition(),
            G4AntiNeutrinoMu::Definition(),
            G4AntiNeutrinoTau::Definition()
    };
}

void TrackingAction::PreUserTrackingAction(const G4Track *track) {
    // ignore neutrinos
    auto *particle = const_cast<G4ParticleDefinition *>(track->GetParticleDefinition());
    if (fParticlesToIgnore.find(particle) != fParticlesToIgnore.end()) {
        // kill
        G4Track *nonConstTrack = const_cast<G4Track *>(track);
        nonConstTrack->SetTrackStatus(fStopAndKill);
        return;
    }


    return;
    // print track info
    G4String particleName = particle->GetParticleName();
    G4String volumeName = track->GetVolume()->GetName();
    G4double stepLength = track->GetStepLength();
    G4double energyDeposit = track->GetTotalEnergy();
    G4double globalTime = track->GetGlobalTime();
    G4double kineticEnergy = track->GetKineticEnergy();
    // position in mm
    G4ThreeVector position = track->GetPosition() / mm;
    const G4ThreeVector &momentum = track->GetMomentumDirection();

    auto positionOrigin = track->GetVertexPosition();
    G4cout << "TrackingAction::PreUserTrackingAction: "
           << "particle=" << particleName << " "
           << "volume=" << volumeName << " "
           << "stepLength=" << G4BestUnit(stepLength, "Length") << " "
           << "energyDeposit=" << G4BestUnit(energyDeposit, "Energy") << " "
           << "globalTime=" << G4BestUnit(globalTime, "Time") << " "
           << "kineticEnergy=" << G4BestUnit(kineticEnergy, "Energy") << " "
           << "position=" << position << " "
           << "momentum=" << momentum << " " << endl;

}

void TrackingAction::PostUserTrackingAction(const G4Track *track) {}
