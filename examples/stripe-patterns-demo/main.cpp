#include "geometrycentral/surface/direction_fields.h"
#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/meshio.h"
#include "geometrycentral/surface/stripe_patterns.h"
#include "geometrycentral/surface/surface_mesh_factories.h"


#include "polyscope/curve_network.h"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"


#include "args.hxx"
#include "imgui.h"

using namespace geometrycentral;
using namespace geometrycentral::surface;

// == Geometry-central data
std::unique_ptr<ManifoldSurfaceMesh> mesh;
std::unique_ptr<VertexPositionGeometry> geometry;

// Polyscope visualization handle
polyscope::SurfaceMesh* psMesh = nullptr;

// Parameters
float stripeFrequency = 40.0;
bool showStripes = true;
bool connectIsolines = true;

// Stripe pattern data
VertexData<Vector2> guideField;
VertexData<double> frequencies;
std::vector<Vector3> isolineVerts;
std::vector<std::array<size_t, 2>> isolineEdges;

void computeStripePattern() {
  // Generate a guiding field
  VertexData<Vector2> guideField = geometrycentral::surface::computeSmoothestVertexDirectionField(*geometry, 2);

  // Compute the stripe pattern
  VertexData<double> frequencies(*mesh, stripeFrequency);
  CornerData<double> periodicFunc;
  FaceData<int> zeroIndices;
  FaceData<int> branchIndices;
  std::tie(periodicFunc, zeroIndices, branchIndices) = computeStripePattern(*geometry, frequencies, guideField);

  // Extract isolines
  std::vector<Vector3> isolineVerts;
  std::vector<std::array<size_t, 2>> isolineEdges;
  std::tie(isolineVerts, isolineEdges) =
      extractPolylinesFromStripePattern(*geometry, periodicFunc, zeroIndices, branchIndices, guideField, false);

  // Visualize
  polyscope::registerCurveNetwork("stripes", isolineVerts, isolineEdges)->setEnabled(showStripes);
}

void myCallback() {
  ImGui::PushItemWidth(100);

  if (ImGui::TreeNode("Stripe Pattern Controls")) {
    bool paramsChanged = false;

    paramsChanged |= ImGui::SliderFloat("Frequency", &stripeFrequency, 1.0f, 50.0f);
    paramsChanged |= ImGui::Checkbox("Connect isolines", &connectIsolines);

    if (paramsChanged) {
      computeStripePattern();
    }

    ImGui::Checkbox("Show stripes", &showStripes);
    if (auto* cn = polyscope::getCurveNetwork("stripes")) {
      cn->setEnabled(showStripes);
    }

    ImGui::TreePop();
  }

  ImGui::PopItemWidth();
}

int main(int argc, char** argv) {
  // Configure the argument parser
  args::ArgumentParser parser("Stripe Patterns Demo");
  args::HelpFlag help(parser, "help", "Display this help message", {'h', "help"});
  args::ValueFlag<std::string> inputMesh(parser, "mesh", "Input mesh file", {"mesh"});

  // Parse args
  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Help&) {
    std::cout << parser;
    return 0;
  } catch (const args::ParseError& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  // Load mesh
  std::string meshFilename = args::get(inputMesh);
  if (meshFilename == "") {
    // Load default mesh if none specified
    return 0;
  } else {
    std::tie(mesh, geometry) = readManifoldSurfaceMesh(meshFilename);
  }

  // Initialize polyscope
  polyscope::init();

  // Register the mesh with polyscope
  psMesh = polyscope::registerSurfaceMesh("input mesh", geometry->inputVertexPositions, mesh->getFaceVertexList(),
                                          polyscopePermutations(*mesh));

  // Initial computation
  computeStripePattern();

  // Set the callback function
  polyscope::state::userCallback = myCallback;

  // Give control to the polyscope gui
  polyscope::show();

  return EXIT_SUCCESS;
}