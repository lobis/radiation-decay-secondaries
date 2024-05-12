
#include <G4UserTrackingAction.hh>
#include <set>
#include <G4ParticleDefinition.hh>


class TrackingAction : public G4UserTrackingAction {
public:
    TrackingAction();

    void PreUserTrackingAction(const G4Track *) override;

    void PostUserTrackingAction(const G4Track *) override;

private:
    std::set<G4ParticleDefinition *> fParticlesToIgnore;
};


