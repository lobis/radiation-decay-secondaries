
#pragma once

#include <G4RunManager.hh>
#include <G4UserRunAction.hh>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

class RunAction : public G4UserRunAction {
public:
    RunAction();

    void BeginOfRunAction(const G4Run *) override;

    void EndOfRunAction(const G4Run *) override;

    static void InsertTrack(const G4Track *track);

    static void SetInputParticle(const std::string &particleName);

    static void SetOutputFilename(const std::string &outputFilename);

    static void SetRequestedPrimaries(int);

    static int GetRequestedPrimaries();

    static void SetRequestedSecondaries(int);

    static int GetRequestedSecondaries();

    static void IncreaseLaunchedPrimaries(const std::string &);

    static unsigned int GetLaunchedPrimaries(bool lock = true);

    static unsigned long long GetSecondariesCount(bool lock = true);

    static std::string GetParticleName() { return inputParticleName; }

private:
    static int requestedPrimaries;
    static int requestedSecondaries;

    static std::map<std::string, double> launchedPrimariesMap;

    static std::string inputFilename;
    static std::string outputFilename;

    static std::mutex inputMutex;
    static std::mutex outputMutex;

    static std::string inputParticleName;
    static TFile *outputFile;

    static TH1D *electronsMinusEnergy;
    static TH1D *electronsPlusEnergy;
    static TH1D *gammasEnergy;
    static TH1D *alphasEnergy;
    static TH1D *neutronsEnergy;

    static TH1D *electronsMinusZenith;
    static TH1D *electronsPlusZenith;
    static TH1D *gammasZenith;
    static TH1D *alphasZenith;
    static TH1D *neutronsZenith;

    static TH2D *electronsMinusEnergyZenith;
    static TH2D *electronsPlusEnergyZenith;
    static TH2D *gammasEnergyZenith;
    static TH2D *alphasEnergyZenith;
    static TH2D *neutronsEnergyZenith;
};


