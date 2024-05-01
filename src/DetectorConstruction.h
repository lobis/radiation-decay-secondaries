
#pragma once

#include <G4GDMLParser.hh>
#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4NistManager.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VisAttributes.hh>
#include <globals.hh>


class DetectorConstruction : public G4VUserDetectorConstruction {

public:
    explicit DetectorConstruction(const std::vector<std::pair<std::string, double>> &configuration);

    G4VPhysicalVolume *Construct() override;

    G4VPhysicalVolume *GetWorld() const { return world; }

    void ConstructSDandField() override;

    static double GetThickness();

private:
    G4VPhysicalVolume *world = nullptr;

    const std::vector<std::pair<std::string, double>> configuration;

    double totalThickness = 0;
};


