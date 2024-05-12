
#include "RunAction.h"
#include "DetectorConstruction.h"

#include <iostream>
#include <TMath.h>
#include <TSystem.h>
#include <filesystem>
#include <numeric>

using namespace std;
using namespace CLHEP;

int RunAction::requestedPrimaries = 0;
int RunAction::requestedSecondaries = 0;

map<string, double> RunAction::launchedPrimariesMap = {};

mutex RunAction::inputMutex;
mutex RunAction::outputMutex;

string RunAction::inputParticleName;
string RunAction::outputFilename;

TFile *RunAction::outputFile = nullptr;

TH1D *RunAction::electronsMinusEnergy = nullptr;
TH1D *RunAction::electronsPlusEnergy = nullptr;
TH1D *RunAction::gammasEnergy = nullptr;
TH1D *RunAction::alphasEnergy = nullptr;
TH1D *RunAction::neutronsEnergy = nullptr;

TH1D *RunAction::electronsMinusZenith = nullptr;
TH1D *RunAction::electronsPlusZenith = nullptr;
TH1D *RunAction::gammasZenith = nullptr;
TH1D *RunAction::alphasZenith = nullptr;
TH1D *RunAction::neutronsZenith = nullptr;

TH2D *RunAction::electronsMinusEnergyZenith = nullptr;
TH2D *RunAction::electronsPlusEnergyZenith = nullptr;
TH2D *RunAction::gammasEnergyZenith = nullptr;
TH2D *RunAction::alphasEnergyZenith = nullptr;
TH2D *RunAction::neutronsEnergyZenith = nullptr;

RunAction::RunAction() : G4UserRunAction() {}

void RunAction::BeginOfRunAction(const G4Run *) {
    lock_guard<std::mutex> lock(mutex);

    if (IsMaster()) {
        if (outputFile != nullptr) {
            outputFile->Close();
            delete outputFile;
        }

        outputFile = new TFile(outputFilename.c_str(), "RECREATE");

        const unsigned int binsEnergyN = 5000;
        const double binsEnergyMin = 1E-4;
        const double binsEnergyMax = 1E2;
        double binsEnergy[binsEnergyN + 1];
        for (int i = 0; i <= binsEnergyN; ++i) {
            binsEnergy[i] = TMath::Power(10, (TMath::Log10(binsEnergyMin) +
                                              i * (TMath::Log10(binsEnergyMax) - TMath::Log10(binsEnergyMin)) /
                                              binsEnergyN));
        }
        const unsigned int binsZenithN = 200;
        const double binsZenithMin = 0;
        const double binsZenithMax = 90;

        electronsMinusEnergy = new TH1D("electron_minus_energy", "Electron (e-) Kinetic Energy (MeV)", binsEnergyN,
                                        binsEnergy);
        electronsMinusEnergy->GetXaxis()->SetTitle("Energy (MeV)");
        electronsMinusEnergy->GetYaxis()->SetTitle("Counts / Bq / cm");

        electronsPlusEnergy = new TH1D("electron_plus_energy", "Electron (e+) Kinetic Energy (MeV)", binsEnergyN,
                                       binsEnergy);
        electronsPlusEnergy->GetXaxis()->SetTitle("Energy (MeV)");
        electronsPlusEnergy->GetYaxis()->SetTitle("Counts / Bq / cm");

        gammasEnergy = new TH1D("gamma_energy", "Gamma Kinetic Energy (MeV)", binsEnergyN, binsEnergy);
        gammasEnergy->GetXaxis()->SetTitle("Energy (MeV)");
        gammasEnergy->GetYaxis()->SetTitle("Counts / Bq / cm");

        alphasEnergy = new TH1D("alpha_energy", "Alpha Kinetic Energy (MeV)", binsEnergyN, binsEnergy);
        alphasEnergy->GetXaxis()->SetTitle("Energy (MeV)");
        alphasEnergy->GetYaxis()->SetTitle("Counts / Bq / cm");

        neutronsEnergy = new TH1D("neutron_energy", "Neutron Kinetic Energy (MeV)", binsEnergyN, binsEnergy);
        neutronsEnergy->GetXaxis()->SetTitle("Energy (MeV)");
        neutronsEnergy->GetYaxis()->SetTitle("Counts / Bq / cm");

        electronsMinusZenith = new TH1D("electron_minus_zenith", "Electron (e-) Zenith Angle (degrees)", binsZenithN,
                                        binsZenithMin,
                                        binsZenithMax);
        electronsMinusZenith->GetXaxis()->SetTitle("Zenith Angle (degrees)");
        electronsMinusZenith->GetYaxis()->SetTitle("Counts / Bq / cm");

        electronsPlusZenith = new TH1D("electron_plus_zenith", "Electron (e+) Zenith Angle (degrees)", binsZenithN,
                                       binsZenithMin,
                                       binsZenithMax);
        electronsPlusZenith->GetXaxis()->SetTitle("Zenith Angle (degrees)");
        electronsPlusZenith->GetYaxis()->SetTitle("Counts / Bq / cm");

        gammasZenith = new TH1D("gamma_zenith", "Gamma Zenith Angle (degrees)", binsZenithN, binsZenithMin,
                                binsZenithMax);
        gammasZenith->GetXaxis()->SetTitle("Zenith Angle (degrees)");
        gammasZenith->GetYaxis()->SetTitle("Counts / Bq / cm");

        alphasZenith = new TH1D("alpha_zenith", "Alpha Zenith Angle (degrees)", binsZenithN, binsZenithMin,
                                binsZenithMax);
        alphasZenith->GetXaxis()->SetTitle("Zenith Angle (degrees)");
        alphasZenith->GetYaxis()->SetTitle("Counts / Bq / cm");

        neutronsZenith = new TH1D("neutron_zenith", "Neutron Zenith Angle (degrees)", binsZenithN, binsZenithMin,
                                  binsZenithMax);
        neutronsZenith->GetXaxis()->SetTitle("Zenith Angle (degrees)");
        neutronsZenith->GetYaxis()->SetTitle("Counts / Bq / cm");

        electronsMinusEnergyZenith = new TH2D("electron_minus_energy_zenith",
                                              "Electron (e-) Kinetic Energy (MeV) vs Zenith Angle (degrees)",
                                              binsEnergyN,
                                              binsEnergy, binsZenithN, binsZenithMin, binsZenithMax);
        electronsMinusEnergyZenith->GetXaxis()->SetTitle("Energy (MeV)");
        electronsMinusEnergyZenith->GetYaxis()->SetTitle("Zenith Angle (degrees)");
        electronsMinusEnergyZenith->GetZaxis()->SetTitle("Counts / Bq / cm");

        electronsPlusEnergyZenith = new TH2D("electron_plus_energy_zenith",
                                             "Electron (e+) Kinetic Energy (MeV) vs Zenith Angle (degrees)",
                                             binsEnergyN,
                                             binsEnergy, binsZenithN, binsZenithMin, binsZenithMax);
        electronsPlusEnergyZenith->GetXaxis()->SetTitle("Energy (MeV)");
        electronsPlusEnergyZenith->GetYaxis()->SetTitle("Zenith Angle (degrees)");
        electronsPlusEnergyZenith->GetZaxis()->SetTitle("Counts / Bq / cm");

        gammasEnergyZenith = new TH2D("gamma_energy_zenith", "Gamma Kinetic Energy (MeV) vs Zenith Angle (degrees)",
                                      binsEnergyN,
                                      binsEnergy,
                                      binsZenithN, binsZenithMin, binsZenithMax);
        gammasEnergyZenith->GetXaxis()->SetTitle("Energy (MeV)");
        gammasEnergyZenith->GetYaxis()->SetTitle("Zenith Angle (degrees)");
        gammasEnergyZenith->GetZaxis()->SetTitle("Counts / Bq / cm");

        alphasEnergyZenith = new TH2D("alpha_energy_zenith", "Alpha Kinetic Energy (MeV) vs Zenith Angle (degrees)",
                                      binsEnergyN,
                                      binsEnergy,
                                      binsZenithN, binsZenithMin, binsZenithMax);
        alphasEnergyZenith->GetXaxis()->SetTitle("Energy (MeV)");
        alphasEnergyZenith->GetYaxis()->SetTitle("Zenith Angle (degrees)");
        alphasEnergyZenith->GetZaxis()->SetTitle("Counts / Bq / cm");

        neutronsEnergyZenith = new TH2D("neutron_energy_zenith",
                                        "Neutron Kinetic Energy (MeV) vs Zenith Angle (degrees)", binsEnergyN,
                                        binsEnergy, binsZenithN, binsZenithMin, binsZenithMax);
        neutronsEnergyZenith->GetXaxis()->SetTitle("Energy (MeV)");
        neutronsEnergyZenith->GetYaxis()->SetTitle("Zenith Angle (degrees)");
        neutronsEnergyZenith->GetZaxis()->SetTitle("Counts / Bq / cm");
    }
}

void RunAction::EndOfRunAction(const G4Run *) {
    if (!isMaster) { return; }

    lock_guard<std::mutex> lockInput(inputMutex);
    lock_guard<std::mutex> lockOutput(outputMutex);

    const auto launchedParticles = GetLaunchedPrimaries(false);
    const auto detectorThickness = DetectorConstruction::GetThickness() * 0.1; // in cm
    const auto scale = 1.0 / launchedParticles / detectorThickness;

    electronsMinusEnergy->Scale(scale);
    electronsPlusEnergy->Scale(scale);
    gammasEnergy->Scale(scale);
    alphasEnergy->Scale(scale);
    neutronsEnergy->Scale(scale);

    electronsMinusZenith->Scale(scale);
    electronsPlusZenith->Scale(scale);
    gammasZenith->Scale(scale);
    alphasZenith->Scale(scale);
    neutronsZenith->Scale(scale);

    electronsMinusEnergyZenith->Scale(scale);
    electronsPlusEnergyZenith->Scale(scale);
    gammasEnergyZenith->Scale(scale);
    alphasEnergyZenith->Scale(scale);
    neutronsEnergyZenith->Scale(scale);

    if (outputFile != nullptr) {
        outputFile->Write();
        outputFile->Close();
        delete outputFile;
        outputFile = nullptr;
    }
}

void RunAction::InsertTrack(const G4Track *track) {
    lock_guard<std::mutex> lock(outputMutex);

    auto *particle = const_cast<G4ParticleDefinition *>(track->GetParticleDefinition());
    G4String particleName = particle->GetParticleName();
    // Energy in MeV
    G4double kineticEnergy = track->GetKineticEnergy() / MeV;
    G4double zenith =
            TMath::ACos(track->GetMomentumDirection().z()) * TMath::RadToDeg();


    if (particleName == "e-") {
        electronsMinusEnergy->Fill(kineticEnergy);
        electronsMinusZenith->Fill(zenith);
        electronsMinusEnergyZenith->Fill(kineticEnergy, zenith);
    } else if (particleName == "e+") {
        electronsPlusEnergy->Fill(kineticEnergy);
        electronsPlusZenith->Fill(zenith);
        electronsPlusEnergyZenith->Fill(kineticEnergy, zenith);
    } else if (particleName == "gamma") {
        gammasEnergy->Fill(kineticEnergy);
        gammasZenith->Fill(zenith);
        gammasEnergyZenith->Fill(kineticEnergy, zenith);
    } else if (particleName == "neutron") {
        neutronsEnergy->Fill(kineticEnergy);
        neutronsZenith->Fill(zenith);
        neutronsEnergyZenith->Fill(kineticEnergy, zenith);
    } else if (particleName == "alpha") {
        alphasEnergy->Fill(kineticEnergy);
        alphasZenith->Fill(zenith);
        alphasEnergyZenith->Fill(kineticEnergy, zenith);
    } else {
        G4cout << "Unknown particle: " << particleName << G4endl;
        return;
    }

    if (requestedSecondaries > 0 && GetSecondariesCount(false) >= requestedSecondaries) {
        G4RunManager::GetRunManager()->AbortRun(true);
    }
}

void RunAction::SetInputParticle(const string &particleName) {
    inputParticleName = particleName;
}

void RunAction::SetOutputFilename(const string &name) {
    outputFilename = name;
}

void RunAction::SetRequestedPrimaries(int newValue) {
    RunAction::requestedPrimaries = newValue;
}

void RunAction::SetRequestedSecondaries(int newValue) {
    RunAction::requestedSecondaries = newValue;
}

int RunAction::GetRequestedPrimaries() {
    return RunAction::requestedPrimaries;
}

int RunAction::GetRequestedSecondaries() {
    return RunAction::requestedSecondaries;
}

unsigned long long RunAction::GetSecondariesCount(bool lock) {
    if (electronsMinusEnergyZenith == nullptr || electronsPlusEnergyZenith == nullptr ||
        gammasEnergyZenith == nullptr || alphasEnergyZenith == nullptr ||
        neutronsEnergyZenith == nullptr) {
        return 0;
    }

    if (lock) {
        outputMutex.lock();
    }
    auto count = electronsMinusEnergyZenith->GetEntries() + electronsPlusEnergyZenith->GetEntries() +
                 gammasEnergyZenith->GetEntries() + alphasEnergyZenith->GetEntries() +
                 neutronsEnergyZenith->GetEntries();
    if (lock) {
        outputMutex.unlock();
    }
    return count;
}

void RunAction::IncreaseLaunchedPrimaries(const string &particleName) {
    lock_guard<std::mutex> lock(inputMutex);
    RunAction::launchedPrimariesMap[particleName]++;
}

unsigned int RunAction::GetLaunchedPrimaries(bool lock) {
    if (lock) {
        inputMutex.lock();
    }
    auto count = std::accumulate(launchedPrimariesMap.begin(), launchedPrimariesMap.end(), 0,
                                 [](unsigned int sum, const auto &entry) { return sum + entry.second; });
    if (lock) {
        inputMutex.unlock();
    }
    return count;
}
