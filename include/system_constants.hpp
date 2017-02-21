/*************************************************************************
 *  Compilation: ./run_main.sh
 *  Execution: ./run_main.sh
 *  Dependencies: None
 *
 * Header file declaring system wide constants.
 *************************************************************************/
#ifndef SYSTEMCONSTANTS_HPP
#define SYSTEMCONSTANTS_HPP

#include <iostream>
#include <map>
#include "emdw.hpp"
#include "discretetable.hpp"
#include "gausscanonical.hpp"
#include "canonical_gaussian_mixture.hpp"
#include "linear_gaussian.hpp"
#include "node.hpp"
#include "graph.hpp"
#include "graph_builder.hpp"
#include "measurement_manager.hpp"
#include "transforms.hpp"

// Function prototypes
void initialiseRCovMat(Matrix<double>& RCov);
void initialiseQCovMat(Matrix<double>& QCov);
void initialiseSensorLocations(std::vector<ColVector<double>>& locations);
void initialiseMeasurementModels(std::vector<rcptr<V2VTransform>>& models, std::vector<ColVector<double>>& locations);
void initialiseLaunchStateMean(std::vector<ColVector<double>>& launchState);
void initialiseLaunchStateCov(std::vector<Matrix<double>>& launchCov);
void initialiseGenericMean(ColVector<double>& genericMean);
void initialiseGenericCov(Matrix<double>& genericCov);
void initialiseGenericWeights(std::vector<double>& weights);
bool initialiseVariables();

// Typedefs
typedef DiscreteTable<unsigned> DT;
typedef GaussCanonical GC;
typedef CanonicalGaussianMixture CGM;
typedef LinearGaussian LG;

namespace mht {
	// Discrete time stepI
	extern const double kTimeStep;

	// Sensor location information
	extern const unsigned short kNumSensors;
	extern std::vector<ColVector<double>> kSensorLocation;

	// Dimension of state and observation vectors
	extern const unsigned short kStateSpaceDim;
	extern const unsigned short kMeasSpaceDim;

	
	// Motion model
	extern rcptr<V2VTransform> kMotionModel;
	extern Matrix<double> kRCovMat;

	// Measurement model
	extern const double kC; // Speed of light
	extern const double kFc; // Carrier frequency
	extern const double kTp; // Sweep period
	extern const double kBw; // Bandwidth

	extern std::vector<rcptr<V2VTransform>> kMeasurementModel;
	extern Matrix<double> kQCovMat;

	// Gaussian mixture pruning parameters
	extern const unsigned kMaxComponents;
	extern const double kThreshold;
	extern const double kMergeDistance;

	
	// Launch locations
	extern std::vector<ColVector<double>> kLaunchStateMean;
	extern std::vector<Matrix<double>> kLaunchStateCov;

	extern ColVector<double> kGenericMean;
	extern Matrix<double> kGenericCov;
	extern std::vector<double> kGenericWeights;

	// Force initialisation
	extern bool init;
} // class

// Variable management
extern emdw::RVIds variables; // Global variables
extern emdw::RVIds vecX;
extern emdw::RVIds vecZ;
extern std::map<unsigned, emdw::RVIds> currentStates;
extern std::map<unsigned, emdw::RVIds> elementsOfX;
extern std::map<unsigned, emdw::RVIds> elementsOfZ;
extern std::map<unsigned, emdw::RVIds> presentAt;
extern emdw::RVIds virtualMeasurementVars;

// Measurement management
extern rcptr<MeasurementManager> manager;
extern unsigned kNumberOfTimeSteps;

// Graph representation
extern std::map<unsigned, std::vector<rcptr<Node>>> stateNodes;
extern std::map<unsigned, std::vector<rcptr<Node>>> measurementNodes;
extern std::map<unsigned, std::vector<rcptr<Factor>>> predMeasurements;
extern std::map<unsigned, std::vector<rcptr<Factor>>> validationRegion;

#endif // SYSTEMCONSTANTS_HPP
