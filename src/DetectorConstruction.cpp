
#include "DetectorConstruction.h"
#include "SensitiveDetector.h"

#include <G4LogicalVolumeStore.hh>
#include <G4NistManager.hh>
#include <G4PhysicalVolumeStore.hh>

#include <random>
#include <G4PVPlacement.hh>
#include <G4RunManager.hh>

using namespace std;
using namespace CLHEP;

DetectorConstruction::DetectorConstruction(const std::vector<std::pair<std::string, double>> &configuration)
        : G4VUserDetectorConstruction(), configuration(configuration) {
        LoadCustomMaterialsFromXML("../materials/materials.xml");
}


G4VPhysicalVolume *DetectorConstruction::Construct() {
    // build a basic detector
    auto nist = G4NistManager::Instance();
    const auto vacuum = nist->FindOrBuildMaterial("G4_Galactic");

     // check all the materials are valid
    for (const auto &[material, thickness]: configuration) {
        if (GetMaterialOrCustom(material) == nullptr) {
            throw runtime_error("Material " + material + " not found");
        }
        if (thickness < 0) {
            throw runtime_error("Thickness cannot be negative");
        }
    }

    constexpr double width = 100.0 * km; // infinite in x and y

    constexpr double detectorThickness = 1.0 * nm;

    auto worldSolid = new G4Box("World", width / 2, width / 2, width / 2);
    auto worldLogical = new G4LogicalVolume(worldSolid, vacuum, "World");
    world = new G4PVPlacement(nullptr, {}, worldLogical, "World", nullptr, false, 0);

    totalThickness = 0;
    for (size_t i = 0; i < configuration.size(); ++i) {
        const auto &config = configuration[i];
        G4Material* material = GetMaterialOrCustom(config.first);
        double thickness = config.second * mm;

        cout << "Layer " << i << ": " << thickness/mm << " mm of " << material->GetName() << endl;

        if (thickness == 0) {
            cout << "Warning: Layer " << i << " has zero thickness, skipping." << endl;
            continue;
        }

        auto solid = new G4Box("Layer" + to_string(i), width / 2, width / 2, thickness / 2);
        auto logical = new G4LogicalVolume(solid, material, "Layer" + to_string(i));
        new G4PVPlacement(nullptr, {0, 0, totalThickness + thickness / 2}, logical, "Layer" + to_string(i),
                          worldLogical, false, 0);

        totalThickness += thickness;
    }

    auto detectorSolid = new G4Box("Detector", width / 2, width / 2, detectorThickness / 2);
    auto detectorLogical = new G4LogicalVolume(detectorSolid, vacuum, "Detector");
    new G4PVPlacement(nullptr, {0, 0, totalThickness + detectorThickness / 2}, detectorLogical, "Detector",
                      worldLogical, false, 0);

    // check for overlaps (not sure if this actually works)
    if (world->CheckOverlaps(1000, 0, true)) {
        throw runtime_error("Overlaps found in geometry");
    }

    return world;
}

void DetectorConstruction::ConstructSDandField() {
    auto detectorLogical = G4LogicalVolumeStore::GetInstance()->GetVolume("Detector");
    auto detector = new SensitiveDetector("Detector");
    SetSensitiveDetector(detectorLogical, detector);
}

double DetectorConstruction::GetThickness() {
    auto detectorConstruction = (DetectorConstruction *) G4RunManager::GetRunManager()->GetUserDetectorConstruction();
    return detectorConstruction->totalThickness;
}

void DetectorConstruction::LoadCustomMaterialsFromXML(const std::string& filename) {
    TXMLEngine xml;
    XMLDocPointer_t doc = xml.ParseFile(filename.c_str());
    if (!doc) {
        throw std::runtime_error("Cannot load materials XML file: " + filename);
    }

    XMLNodePointer_t root = xml.DocGetRootElement(doc);
    if (std::string(xml.GetNodeName(root)) != "materials") {
        throw std::runtime_error("Root node must be <materials>");
    }

    G4NistManager* nist = G4NistManager::Instance();

    for (XMLNodePointer_t matNode = xml.GetChild(root); matNode != nullptr; matNode = xml.GetNext(matNode)) {
        if (std::string(xml.GetNodeName(matNode)) != "material") continue;

        std::string name = xml.GetAttr(matNode, "name");
        std::string stateStr = xml.GetAttr(matNode, "state");

        G4State state = kStateUndefined;
        if (stateStr == "solid") state = kStateSolid;
        else if (stateStr == "liquid") state = kStateLiquid;
        else if (stateStr == "gas") state = kStateGas;

        // density
        double density = -1;
        std::string unitStr;
        for (XMLNodePointer_t dNode = xml.GetChild(matNode); dNode != nullptr; dNode = xml.GetNext(dNode)) {
            if (std::string(xml.GetNodeName(dNode)) == "D") {
                density = atof(xml.GetAttr(dNode, "value"));
                unitStr = xml.GetAttr(dNode, "unit");
                break;
            }
        }

        if (density < 0 || unitStr.empty()) {
            throw std::runtime_error("Material " + name + " must have <D value=\"...\" unit=\"...\"/> defined.");
        }

        
        double densityFactor = 1.0;
        if (unitStr == "g/cm3") densityFactor = g/cm3;
        else if (unitStr == "kg/m3") densityFactor = kg/m3;
        else throw std::runtime_error("Unknown density unit: " + unitStr);

        //G4Material* material = new G4Material(name, density * densityFactor, 0, state);

        std::vector<std::pair<G4Material*, double>> materialComponents;
        std::vector<std::pair<G4Element*, double>> elementComponents;
        double fractionSum = 0.0;

        for (XMLNodePointer_t compNode = xml.GetChild(matNode); compNode != nullptr; compNode = xml.GetNext(compNode)) {
            if (std::string(xml.GetNodeName(compNode)) != "component") continue;

            const char* elementAttr = xml.GetAttr(compNode, "element");
            const char* materialAttr = xml.GetAttr(compNode, "material");
            const char* fractionAttr = xml.GetAttr(compNode, "fraction");

            if (!fractionAttr) {
            throw std::runtime_error("Component in material " + name + " is missing fraction attribute.");
            }

            double fraction = atof(fractionAttr);
            fractionSum += fraction;

            if (elementAttr) {
            auto element = nist->FindOrBuildElement(elementAttr);
            if (!element) throw std::runtime_error("Element " + std::string(elementAttr) + " not found");
            elementComponents.emplace_back(element, fraction);
            } else if (materialAttr) {
            G4Material* baseMat = nullptr;

            if (nist->FindOrBuildMaterial(materialAttr, false)) {
                baseMat = nist->FindOrBuildMaterial(materialAttr);
            } else if (customMaterials.count(materialAttr)) {
                baseMat = customMaterials[materialAttr];
            } else {
                throw std::runtime_error("Material " + std::string(materialAttr) + " not found");
            }

            materialComponents.emplace_back(baseMat, fraction);
            } else {
            throw std::runtime_error("Component in material " + name + " must specify either element or material");
            }
        }

        // validate the fractions
        const double epsilon = 1e-6;
        if (std::abs(fractionSum - 1.0) > epsilon) {
            std::ostringstream oss;
            oss << "Material " << name << ": fractions must sum to 1.0 (sum is " << fractionSum << ")";
            throw std::runtime_error(oss.str());
        }

        // create material adding components
        int nComponents = elementComponents.size() + materialComponents.size();
        G4Material* material = new G4Material(name, density * densityFactor, nComponents, state);
        for (const auto& [elem, frac] : elementComponents) {
            material->AddElement(elem, frac);
        }
        for (const auto& [mat, frac] : materialComponents) {
            material->AddMaterial(mat, frac);
        }

        customMaterials[name] = material;
        G4cout << "Custom material loaded: " << name << G4endl;
    }

    xml.FreeDoc(doc);
}


G4Material* DetectorConstruction::GetMaterialOrCustom(const std::string& name) {
    auto nist = G4NistManager::Instance();
    G4Material* material = nist->FindOrBuildMaterial(name, false);

    if (!material) {
        auto it = customMaterials.find(name);
        if (it != customMaterials.end()) {
            material = it->second;
        }
    }

    if (!material) {
        throw std::runtime_error("Material '" + name + "' not found in NIST or custom definitions.");
    }

    G4cout << "\n[Material Info] Selected material: " << material->GetName() << G4endl;
    G4cout << "  Density: " << material->GetDensity() / (g/cm3) << G4endl;
    G4cout << "  State: " << material->GetState() << G4endl;
    G4cout << "  Number of components: " << material->GetNumberOfElements() << G4endl;

    for (size_t i = 0; i < material->GetNumberOfElements(); ++i) {
        const G4Element* elem = material->GetElement(i);
        double frac = material->GetFractionVector()[i];
        G4cout << "    Component " << i << ": " << elem->GetName()
               << " (" << elem->GetSymbol() << "), fraction: " << frac << G4endl;
    }

    G4cout << "----------------------------------------------\n" << G4endl;

    return material;
}

