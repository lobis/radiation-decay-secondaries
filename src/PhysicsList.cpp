
#include "PhysicsList.h"

#include <G4DecayPhysics.hh>
#include <G4EmExtraPhysics.hh>
#include <G4EmStandardPhysics.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4HadronElasticPhysics.hh>
#include <G4HadronPhysicsFTFP_BERT.hh>
#include <G4IonPhysics.hh>
#include <G4NeutronTrackingCut.hh>
#include <G4RadioactiveDecayPhysics.hh>
#include <G4StoppingPhysics.hh>
#include <G4VModularPhysicsList.hh>
#include <G4EmLivermorePhysics.hh>
#include <G4EmLivermorePolarizedPhysics.hh>
#include <G4EmPenelopePhysics.hh>
#include <G4HadronElasticPhysicsHP.hh>
#include <G4IonBinaryCascadePhysics.hh>
#include <G4HadronPhysicsQGSP_BIC_HP.hh>
#include <G4NuclideTable.hh>
#include <G4PhysListUtil.hh>
#include <G4EmParameters.hh>
#include <G4DeexPrecoParameters.hh>
#include <G4NuclearLevelData.hh>
#include <G4Radioactivation.hh>
#include <G4LossTableManager.hh>
#include <G4UAtomicDeexcitation.hh>

using namespace std;
using namespace CLHEP;

// https://github.com/Geant4/geant4/blob/master/examples/extended/radioactivedecay/rdecay01/src/PhysicsList.cc

PhysicsList::PhysicsList() : G4VModularPhysicsList() {
    G4PhysListUtil::InitialiseParameters();

    SetVerboseLevel(1);

    const G4double meanLife = 1 * year  ;
    G4NuclideTable::GetInstance()->SetMeanLifeThreshold(meanLife);
    G4NuclideTable::GetInstance()->SetLevelTolerance(1.0 * eV);

    G4EmParameters::Instance()->SetDefaults();
    G4EmParameters::Instance()->SetAugerCascade(true);
    G4EmParameters::Instance()->SetDeexcitationIgnoreCut(true);

    G4DeexPrecoParameters *deex =
            G4NuclearLevelData::GetInstance()->GetParameters();
    deex->SetCorrelatedGamma(false);
    deex->SetStoreAllLevels(true);
    deex->SetInternalConversionFlag(true);
    deex->SetIsomerProduction(true);
    deex->SetMaxLifeTime(meanLife);

    SetDefaultCutValue(1 * mm);

    RegisterPhysics(new G4DecayPhysics());
    RegisterPhysics(new G4RadioactiveDecayPhysics());
    // RegisterPhysics(new G4EmExtraPhysics());
    RegisterPhysics(new G4IonBinaryCascadePhysics());
    RegisterPhysics(new G4HadronPhysicsQGSP_BIC_HP());
    RegisterPhysics(new G4HadronElasticPhysicsHP());
    RegisterPhysics(new G4IonPhysics());

    // Neutron tracking cut
    RegisterPhysics(new G4NeutronTrackingCut());

    // RegisterPhysics(new G4EmLivermorePhysics());
    RegisterPhysics(new G4EmStandardPhysics_option4());
}

void PhysicsList::ConstructProcess() {
    AddTransportation();

    G4RadioactiveDecay *radioactiveDecay = new G4RadioactiveDecay();

    radioactiveDecay->SetThresholdForVeryLongDecayTime(pow(10,12) * CLHEP::year);

    G4bool ARMflag = false;
    radioactiveDecay->SetARM(ARMflag);        //Atomic Rearangement

    // EM physics constructor is not used in this example, so
    // it is needed to instantiate and to initialize atomic deexcitation
    //
    G4LossTableManager *man = G4LossTableManager::Instance();
    G4VAtomDeexcitation *deex = man->AtomDeexcitation();
    if (nullptr == deex) {
        deex = new G4UAtomicDeexcitation();
        man->SetAtomDeexcitation(deex);
    }
    deex->InitialiseAtomicDeexcitation();

    G4PhysicsListHelper *ph = G4PhysicsListHelper::GetPhysicsListHelper();
    ph->RegisterProcess(radioactiveDecay, G4GenericIon::GenericIon());

    G4VModularPhysicsList::ConstructProcess();
}
