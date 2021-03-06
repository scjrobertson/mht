/*************************************************************************
 *  Compilation: ./run_main.sh
 *  Execution: ./run_main.sh
 *  Dependencies: None
 *
 * Source file for the Canonical Gaussian Mixture implementation.
 *************************************************************************/
#include <vector>
#include <map>
#include <iostream>
#include <math.h>
#include <limits>
#include "sortindices.hpp"
#include "genvec.hpp"
#include "genmat.hpp"
#include "emdw.hpp"
#include "matops.hpp"
#include "vecset.hpp"
#include "gausscanonical.hpp"
#include "canonical_gaussian_mixture.hpp"

// Default operators
rcptr<FactorOperator> defaultInplaceNormalizerCGM = uniqptr<FactorOperator>(new InplaceNormalizeCGM());
rcptr<FactorOperator> defaultNormalizerCGM = uniqptr<FactorOperator>(new NormalizeCGM());
rcptr<FactorOperator> defaultInplaceAbsorberCGM = uniqptr<FactorOperator>(new InplaceAbsorbCGM());
rcptr<FactorOperator> defaultAbsorberCGM = uniqptr<FactorOperator>(new AbsorbCGM());
rcptr<FactorOperator> defaultInplaceCancellerCGM = uniqptr<FactorOperator>(new InplaceCancelCGM());
rcptr<FactorOperator> defaultCancellerCGM = uniqptr<FactorOperator>(new CancelCGM());
rcptr<FactorOperator> defaultMarginalizerCGM = uniqptr<FactorOperator>(new MarginalizeCGM());
rcptr<FactorOperator> defaultObserveReducerCGM = uniqptr<FactorOperator>(new ObserveAndReduceCGM());
rcptr<FactorOperator> defaultInplaceWeakDamperCGM = uniqptr<FactorOperator>(new InplaceWeakDampingCGM());

CanonicalGaussianMixture::CanonicalGaussianMixture(
		const emdw::RVIds& vars,
		bool presorted,
		const unsigned maxComponents,
		const double threshold,
		const double unionDistance,
		const rcptr<FactorOperator>& inplaceNormalizer,
		const rcptr<FactorOperator>& normalizer,
		const rcptr<FactorOperator>& inplaceAbsorber,
		const rcptr<FactorOperator>& absorber,
		const rcptr<FactorOperator>& inplaceCanceller,
		const rcptr<FactorOperator>& canceller,
		const rcptr<FactorOperator>& marginalizer,
		const rcptr<FactorOperator>& observerAndReducer,
		const rcptr<FactorOperator>& inplaceDamper)
			: vars_(vars.size()),
			maxComp_(maxComponents),
			threshold_(threshold),
			unionDistance_(unionDistance),
			inplaceNormalizer_(inplaceNormalizer),
			normalizer_(normalizer),
			inplaceAbsorber_(inplaceAbsorber),
			absorber_(absorber),
			inplaceCanceller_(inplaceCanceller),
			canceller_(canceller),
			marginalizer_(marginalizer),
			observeAndReducer_(observerAndReducer),
			inplaceDamper_(inplaceDamper)
		{
	// Default operator intialisation
	if (!inplaceNormalizer_) { inplaceNormalizer_ = defaultInplaceNormalizerCGM; }
	if (!normalizer_) { normalizer_ = defaultNormalizerCGM; }
	if (!inplaceAbsorber_) { inplaceAbsorber_ = defaultInplaceAbsorberCGM; }
	if (!absorber_) { absorber_ = defaultAbsorberCGM; }
	if (!inplaceCanceller_) { inplaceCanceller_ = defaultInplaceCancellerCGM; }
	if (!canceller_) { canceller_ = defaultCancellerCGM; }
	if (!marginalizer_) { marginalizer_ = defaultMarginalizerCGM; }
	if (!observeAndReducer_) { observeAndReducer_ = defaultObserveReducerCGM; }
	if (!inplaceDamper_) { inplaceDamper_ = defaultInplaceWeakDamperCGM; }

	// Ensure the higher level description is sorted.
	if (presorted || !vars.size()) {
		vars_ = vars;
	} else {
		std::vector<size_t> sorted = sortIndices(vars, std::less<unsigned>() );
		vars_ = extract<unsigned>(vars, sorted);
	}

	// Create a mixture with a single vacuous component.
	comps_.clear();
	comps_.push_back( uniqptr<Factor> ( new GaussCanonical(vars_, true ) ) );
	N_ = 1;
} // Default Constructor

CanonicalGaussianMixture::CanonicalGaussianMixture(
		const emdw::RVIds& vars,
		const std::vector<double>& weights,
		const std::vector<ColVector<double>>& means,
		const std::vector<Matrix<double>>& covs,
		bool presorted,
		const unsigned maxComponents,
		const double threshold,
		const double unionDistance,
		const rcptr<FactorOperator>& inplaceNormalizer,
		const rcptr<FactorOperator>& normalizer,
		const rcptr<FactorOperator>& inplaceAbsorber,
		const rcptr<FactorOperator>& absorber,
		const rcptr<FactorOperator>& inplaceCanceller,
		const rcptr<FactorOperator>& canceller,
		const rcptr<FactorOperator>& marginalizer,
		const rcptr<FactorOperator>& observerAndReducer,
		const rcptr<FactorOperator>& inplaceDamper)
			: vars_(vars.size()),
			comps_(weights.size()),
			N_(weights.size()),
			maxComp_(maxComponents),
			threshold_(threshold),
			unionDistance_(unionDistance),
			normalizer_(normalizer),
			inplaceAbsorber_(inplaceAbsorber),
			absorber_(absorber),
			inplaceCanceller_(inplaceCanceller),
			canceller_(canceller),
			marginalizer_(marginalizer),
			observeAndReducer_(observerAndReducer),
			inplaceDamper_(inplaceDamper)
		{

	// Default operator intialisation
	if (!inplaceNormalizer_) { inplaceNormalizer_ = defaultInplaceNormalizerCGM; }
	if (!normalizer_) { normalizer_ = defaultNormalizerCGM; }
	if (!inplaceAbsorber_) { inplaceAbsorber_ = defaultInplaceAbsorberCGM; }
	if (!absorber_) { absorber_ = defaultAbsorberCGM; }
	if (!inplaceCanceller_) { inplaceCanceller_ = defaultInplaceCancellerCGM; }
	if (!canceller_) { canceller_ = defaultCancellerCGM; }
	if (!marginalizer_) { marginalizer_ = defaultMarginalizerCGM; }
	if (!observeAndReducer_) { observeAndReducer_ = defaultObserveReducerCGM; }
	if (!inplaceDamper_) { inplaceDamper_ = defaultInplaceWeakDamperCGM; }

	// A quick check
	ASSERT( (means.size() == N_) && (covs.size() == N_),
			"weights.size() = " << weights.size() << ", but means.size() = " <<
			means.size() << "and covs.size() = " << covs.size() );

	// Convert from Covariance to Canonical form, this is done upfront
	// as using adjustMass after initialisation is more expensive.
	for (unsigned i = 0; i < N_; i++) {
		int fail = 0;
		double detK = 0.0;
		Matrix<double> K = inv(covs[i], detK, fail);
		if (fail) printf("Could not invert cov[%d] at line number %d in file %s\n", i, __LINE__, __FILE__);

		ColVector<double> h = K*means[i];
		/*
		double g = -0.5*( means[i].transpose()*h + vars.size()*log( 2*M_PI ) 
			- log(detK) ) + log(weights[i]);
		*/

		comps_[i] = uniqptr<Factor> ( new GaussCanonical(vars, K, h, log(weights[i]), false) );
	}

	// Make the sure high level description is sorted.
	vars_ = comps_[0]->getVars();
} // Covariance constructor

CanonicalGaussianMixture::CanonicalGaussianMixture(
		const emdw::RVIds& vars,
		const std::vector<Matrix<double>>& prec,
		const std::vector<ColVector<double>>& info,
		const std::vector<double>& g,
		bool presorted,
		const unsigned maxComponents,
		const double threshold,
		const double unionDistance,
		const rcptr<FactorOperator>& inplaceNormalizer,
		const rcptr<FactorOperator>& normalizer,
		const rcptr<FactorOperator>& inplaceAbsorber,
		const rcptr<FactorOperator>& absorber,
		const rcptr<FactorOperator>& inplaceCanceller,
		const rcptr<FactorOperator>& canceller,
		const rcptr<FactorOperator>& marginalizer,
		const rcptr<FactorOperator>& observerAndReducer,
		const rcptr<FactorOperator>& inplaceDamper)
			: vars_(vars.size()),
			comps_(g.size()),
			N_(g.size()),
			maxComp_(maxComponents),
			threshold_(threshold),
			unionDistance_(unionDistance),
			inplaceNormalizer_(inplaceNormalizer),
			normalizer_(normalizer),
			inplaceAbsorber_(inplaceAbsorber),
			absorber_(absorber),
			inplaceCanceller_(inplaceCanceller),
			canceller_(canceller),
			marginalizer_(marginalizer),
			observeAndReducer_(observerAndReducer),
			inplaceDamper_(inplaceDamper)
		{

	// Default operator intialisation
	if (!inplaceNormalizer_) { inplaceNormalizer_ = defaultInplaceNormalizerCGM; }
	if (!normalizer_) { normalizer_ = defaultNormalizerCGM; }
	if (!inplaceAbsorber_) { inplaceAbsorber_ = defaultInplaceAbsorberCGM; }
	if (!absorber_) { absorber_ = defaultAbsorberCGM; }
	if (!inplaceCanceller_) { inplaceCanceller_ = defaultInplaceCancellerCGM; }
	if (!canceller_) { canceller_ = defaultCancellerCGM; }
	if (!marginalizer_) { marginalizer_ = defaultMarginalizerCGM; }
	if (!observeAndReducer_) { observeAndReducer_ = defaultObserveReducerCGM; }
	if (!inplaceDamper_) { inplaceDamper_ = defaultInplaceWeakDamperCGM; }

	// A quick check
	ASSERT( (info.size() == N_) && (prec.size() == N_),
			"g.size() = " << N_ << ", but info.size() = " <<
			info.size() << "and prec.size() = " << prec.size() );

	for (unsigned i = 0; i < N_; i++) {
		comps_[i] = uniqptr<Factor> ( new GaussCanonical(vars, prec[i], info[i], g[i], false ) );
	}

	// Make the sure high level description is sorted.
	vars_ = comps_[0]->getVars();
} // Canonical constructor

CanonicalGaussianMixture::CanonicalGaussianMixture(
		const emdw::RVIds& vars,
		const std::vector<rcptr<Factor>>& components,
		bool presorted,
		const unsigned maxComponents,
		const double threshold,
		const double unionDistance,
		const rcptr<FactorOperator>& inplaceNormalizer,
		const rcptr<FactorOperator>& normalizer,
		const rcptr<FactorOperator>& inplaceAbsorber,
		const rcptr<FactorOperator>& absorber,
		const rcptr<FactorOperator>& inplaceCanceller,
		const rcptr<FactorOperator>& canceller,
		const rcptr<FactorOperator>& marginalizer,
		const rcptr<FactorOperator>& observerAndReducer,
		const rcptr<FactorOperator>& inplaceDamper
		) 
			: vars_(vars.size()),
			comps_(components.size()),
			N_(components.size()),
			maxComp_(maxComponents),
			threshold_(threshold),
			unionDistance_(unionDistance),
			inplaceNormalizer_(inplaceNormalizer),
			normalizer_(normalizer),
			inplaceAbsorber_(inplaceAbsorber),
			absorber_(absorber),
			inplaceCanceller_(inplaceCanceller),
			canceller_(canceller),
			marginalizer_(marginalizer),
			observeAndReducer_(observerAndReducer),
			inplaceDamper_(inplaceDamper)
		{
	
	// Default operator intialisation
	if (!inplaceNormalizer_) { inplaceNormalizer_ = defaultInplaceNormalizerCGM; }
	if (!normalizer_) { normalizer_ = defaultNormalizerCGM; }
	if (!inplaceAbsorber_) { inplaceAbsorber_ = defaultInplaceAbsorberCGM; }
	if (!absorber_) { absorber_ = defaultAbsorberCGM; }
	if (!inplaceCanceller_) { inplaceCanceller_ = defaultInplaceCancellerCGM; }
	if (!canceller_) { canceller_ = defaultCancellerCGM; }
	if (!marginalizer_) { marginalizer_ = defaultMarginalizerCGM; }
	if (!observeAndReducer_) { observeAndReducer_ = defaultObserveReducerCGM; }
	if (!inplaceDamper_) { inplaceDamper_ = defaultInplaceWeakDamperCGM; }
	
	// Make the sure high level description is sorted.
	if (presorted || !vars.size()) {
		vars_ = vars;	
	} else {
		std::vector<size_t> sorted = sortIndices(vars, std::less<unsigned>() );
		vars_ = extract<unsigned>(vars, sorted);
	}

	for (unsigned i = 0; i < N_; i++) {
		ASSERT( vars == components[i]->getVars(), vars << " != " << components[i]->getVars()
				<< ". All components must be distributions in " << vars);
		
		comps_[i] = uniqptr<Factor>(components[i]->copy());
	}
} // Component constructor

CanonicalGaussianMixture::CanonicalGaussianMixture(
		const rcptr<Factor>& xFPtr,
		const Matrix<double>& A,
		const emdw::RVIds& newVars,
		const Matrix<double>& Q,
		bool presorted,
		const unsigned maxComponents,
		const double threshold,
		const double unionDistance,
		const rcptr<FactorOperator>& inplaceNormalizer,
		const rcptr<FactorOperator>& normalizer,
		const rcptr<FactorOperator>& inplaceAbsorber,
		const rcptr<FactorOperator>& absorber,
		const rcptr<FactorOperator>& inplaceCanceller,
		const rcptr<FactorOperator>& canceller,
		const rcptr<FactorOperator>& marginalizer,
		const rcptr<FactorOperator>& observerAndReducer,
		const rcptr<FactorOperator>& inplaceDamper
		) 
			: maxComp_(maxComponents),
			threshold_(threshold),
			unionDistance_(unionDistance),
			inplaceNormalizer_(inplaceNormalizer),
			normalizer_(normalizer),
			inplaceAbsorber_(inplaceAbsorber),
			absorber_(absorber),
			inplaceCanceller_(inplaceCanceller),
			canceller_(canceller),
			marginalizer_(marginalizer),
			observeAndReducer_(observerAndReducer),
			inplaceDamper_(inplaceDamper)		
		{

	// Default operator intialisation
	if (!inplaceNormalizer_) { inplaceNormalizer_ = defaultInplaceNormalizerCGM; }
	if (!normalizer_) { normalizer_ = defaultNormalizerCGM; }
	if (!inplaceAbsorber_) { inplaceAbsorber_ = defaultInplaceAbsorberCGM; }
	if (!absorber_) { absorber_ = defaultAbsorberCGM; }
	if (!inplaceCanceller_) { inplaceCanceller_ = defaultInplaceCancellerCGM; }
	if (!canceller_) { canceller_ = defaultCancellerCGM; }
	if (!marginalizer_) { marginalizer_ = defaultMarginalizerCGM; }
	if (!observeAndReducer_) { observeAndReducer_ = defaultObserveReducerCGM; }
	if (!inplaceDamper_) { inplaceDamper_ = defaultInplaceWeakDamperCGM; }

	// Get the old mixture components.
	rcptr<CanonicalGaussianMixture> cgm = std::dynamic_pointer_cast<CanonicalGaussianMixture>(xFPtr);
	std::vector<rcptr<Factor>> oldComps = cgm->getComponents();
	
	// Allocate the new mixture.
	N_ = oldComps.size();
	comps_ = std::vector<rcptr<Factor>>(N_);

	// Put each component through the linear transform.
	for (unsigned i = 0; i < N_; i++) {
		comps_[i] = uniqptr<Factor>(new GaussCanonical(oldComps[i].get(), A, newVars, Q, false ) );
	}

	// Make the new variables are sorted in CanonicalGaussianMixture
	vars_ = comps_[0]->getVars();
} // Linear Gaussian constructor

CanonicalGaussianMixture::CanonicalGaussianMixture(
		const rcptr<Factor>& xFPtr,
		const rcptr<V2VTransform>& transform,
		const emdw::RVIds& newVars,
		const Matrix<double>& Q,
		bool presorted,
		const unsigned maxComponents,
		const double threshold,
		const double unionDistance,
		const rcptr<FactorOperator>& inplaceNormalizer,
		const rcptr<FactorOperator>& normalizer,
		const rcptr<FactorOperator>& inplaceAbsorber,
		const rcptr<FactorOperator>& absorber,
		const rcptr<FactorOperator>& inplaceCanceller,
		const rcptr<FactorOperator>& canceller,
		const rcptr<FactorOperator>& marginalizer,
		const rcptr<FactorOperator>& observerAndReducer,
		const rcptr<FactorOperator>& inplaceDamper
		) 
			: maxComp_(maxComponents),
			threshold_(threshold),
			unionDistance_(unionDistance),
			inplaceNormalizer_(inplaceNormalizer),
			normalizer_(normalizer),
			inplaceAbsorber_(inplaceAbsorber),
			absorber_(absorber),
			inplaceCanceller_(inplaceCanceller),
			canceller_(canceller),
			marginalizer_(marginalizer),
			observeAndReducer_(observerAndReducer),
			inplaceDamper_(inplaceDamper)
		{

	// Default operator intialisation
	if (!inplaceNormalizer_) { inplaceNormalizer_ = defaultInplaceNormalizerCGM; }
	if (!normalizer_) { normalizer_ = defaultNormalizerCGM; }
	if (!inplaceAbsorber_) { inplaceAbsorber_ = defaultInplaceAbsorberCGM; }
	if (!absorber_) { absorber_ = defaultAbsorberCGM; }
	if (!inplaceCanceller_) { inplaceCanceller_ = defaultInplaceCancellerCGM; }
	if (!canceller_) { canceller_ = defaultCancellerCGM; }
	if (!marginalizer_) { marginalizer_ = defaultMarginalizerCGM; }
	if (!observeAndReducer_) { observeAndReducer_ = defaultObserveReducerCGM; }
	if (!inplaceDamper_) { inplaceDamper_ = defaultInplaceWeakDamperCGM; }

	// Get the old mixture components.
	rcptr<CanonicalGaussianMixture> cgm = std::dynamic_pointer_cast<CanonicalGaussianMixture>(xFPtr);
	std::vector<rcptr<Factor>> oldComps = cgm->getComponents();
	
	// Allocate the new mixture.
	N_ = oldComps.size();
	comps_ = std::vector<rcptr<Factor>>(N_);

	// Put each component through the transform.
	for (unsigned i = 0; i < N_; i++) {
		comps_[i] = uniqptr<Factor>(new GaussCanonical(oldComps[i].get(), *transform, newVars, Q, false ) );
	}

	// Make the new variables are sorted in CanonicalGaussianMixture
	vars_ = comps_[0]->getVars();	
} // Non-linear Gaussian constructor

CanonicalGaussianMixture::~CanonicalGaussianMixture() {} // Default Destructor

unsigned CanonicalGaussianMixture::configure(unsigned) {
	std::cout << "NIY" << std::endl;
	return true;
} // configure()

unsigned CanonicalGaussianMixture::classSpecificConfigure(
		const emdw::RVIds& vars,
		const std::vector<rcptr<Factor>>& components,
		bool presorted,
		const unsigned maxComponents,
		const double threshold,
		const double unionDistance,
		const rcptr<FactorOperator>& inplaceNormalizer,
		const rcptr<FactorOperator>& normalizer,
		const rcptr<FactorOperator>& inplaceAbsorber,
		const rcptr<FactorOperator>& absorber,
		const rcptr<FactorOperator>& inplaceCanceller,
		const rcptr<FactorOperator>& canceller,
		const rcptr<FactorOperator>& marginalizer,
		const rcptr<FactorOperator>& observerAndReducer,
		const rcptr<FactorOperator>& inplaceDamper
		) {
	// Destroy existing ...
	this->~CanonicalGaussianMixture();

	// .. and begin anew!
	new(this) CanonicalGaussianMixture(
			vars,
			components,
			presorted,
			maxComponents,
			threshold,
			unionDistance,
			inplaceNormalizer,
			normalizer,
			inplaceAbsorber,
			absorber,
			inplaceCanceller,
			canceller,
			marginalizer,
			observerAndReducer,
			inplaceDamper);
	
	return 1;
} // classSpecificConfigure()


//------------------Family 1: Normalization

inline void CanonicalGaussianMixture::inplaceNormalize(FactorOperator* procPtr) {
	if (procPtr) dynamicInplaceApply(procPtr, this);
	else dynamicInplaceApply(inplaceNormalizer_.get(), this);
} // inplaceNormalize()

inline uniqptr<Factor> CanonicalGaussianMixture::normalize(FactorOperator* procPtr) const {
	if (procPtr) return uniqptr<Factor>(dynamicApply(procPtr, this));
	else return uniqptr<Factor>(dynamicApply(normalizer_.get(), this));
} // normalize()

//------------------Family 2: Absorbtion, Cancellation

inline void CanonicalGaussianMixture::inplaceAbsorb(const Factor* rhsPtr, FactorOperator* procPtr) {
	if (procPtr) dynamicInplaceApply(procPtr, this, rhsPtr);
	else dynamicInplaceApply(inplaceAbsorber_.get(), this, rhsPtr);
} // inplaceAbsorb()

inline uniqptr<Factor> CanonicalGaussianMixture::absorb(const Factor* rhsPtr, FactorOperator* procPtr) const {
	if (procPtr) return uniqptr<Factor> (dynamicApply(procPtr, this, rhsPtr));
	else return uniqptr<Factor> (dynamicApply(absorber_.get(), this, rhsPtr));
} // absorb()

inline void CanonicalGaussianMixture::inplaceCancel(const Factor* rhsPtr, FactorOperator* procPtr) {
	if (procPtr) dynamicInplaceApply(procPtr, this, rhsPtr);
	else dynamicInplaceApply(inplaceCanceller_.get(), this, rhsPtr);
} // inplaceCancel()

inline uniqptr<Factor> CanonicalGaussianMixture::cancel(const Factor* rhsPtr, FactorOperator* procPtr) const {
	if (procPtr) return uniqptr<Factor> (dynamicApply(procPtr, this, rhsPtr));
	else return uniqptr<Factor> (dynamicApply(canceller_.get(), this, rhsPtr));
} // cancel()

//------------------Family 4: Marginalization

inline uniqptr<Factor> CanonicalGaussianMixture::marginalize(const emdw::RVIds& variablesToKeep, 
		bool presorted, FactorOperator* procPtr) const {
	if (procPtr) return uniqptr<Factor> (dynamicApply(procPtr, this, variablesToKeep, presorted));
	else return uniqptr<Factor> (dynamicApply(marginalizer_.get(), this, variablesToKeep, presorted));
} // marginalize()

//------------------Family 4: ObserveAndReduce

inline uniqptr<Factor> CanonicalGaussianMixture::observeAndReduce( const emdw::RVIds& variables,
		const emdw::RVVals& assignedVals, bool presorted, FactorOperator* procPtr) const {
	if (procPtr) return uniqptr<Factor> (dynamicApply(procPtr, this, variables, assignedVals, presorted));
	else return uniqptr<Factor> (dynamicApply(observeAndReducer_.get(), this, variables, assignedVals, presorted));
} // observeAndReduce()


//------------------Family 4: Inplace Weak Damping

// TODO: Complete this!!!
double CanonicalGaussianMixture::inplaceDampen(const Factor* oldMsg, double df, FactorOperator* procPtr) {
	if (procPtr) return dynamicInplaceApply(procPtr, this, oldMsg, df);
	else return dynamicInplaceApply(inplaceDamper_.get(), this, oldMsg, df); 
} // inplaceDampen()

//------------------Other required virtual methods

CanonicalGaussianMixture* CanonicalGaussianMixture::copy(const emdw::RVIds& newVars, bool presorted) const {
	if (newVars.size()) {
		std::vector<rcptr<Factor>> components;
		for (rcptr<Factor> i : comps_) components.push_back( uniqptr<Factor> (i->copy(newVars, presorted) ) );

		// Copy components onto new scope
		return new CanonicalGaussianMixture(
				newVars,
				components,
				true,
				maxComp_,
				threshold_,
				unionDistance_,
				inplaceNormalizer_,
				normalizer_,
				inplaceAbsorber_,
				absorber_,
				inplaceCanceller_,
				canceller_,
				marginalizer_,
				observeAndReducer_,
				inplaceDamper_);
	} 
	return new CanonicalGaussianMixture(*this);
} // copy()

CanonicalGaussianMixture* CanonicalGaussianMixture::vacuousCopy(const emdw::RVIds& selectedVars, bool presorted) const {
	return new CanonicalGaussianMixture(selectedVars);
} // vacuousCopy()

bool CanonicalGaussianMixture::isEqual(const Factor* rhsPtr) const { return true; } // isEqual()

unsigned CanonicalGaussianMixture::noOfVars() const { return vars_.size(); } // noOfVars()

emdw::RVIds CanonicalGaussianMixture::getVars() const { return vars_; } // getVars()

emdw::RVIdType CanonicalGaussianMixture::getVar(unsigned varNo) const { return vars_[varNo]; } // getVar()

//TODO: Complete this!!!
std::istream& CanonicalGaussianMixture::txtRead(std::istream& file) { return file; } // txtRead()

//TODO: Complete this!!
std::ostream& CanonicalGaussianMixture::txtWrite(std::ostream& file) const { 
	std::vector<rcptr<Factor>> components = getComponents();
	
	for (unsigned i = 0; i < N_; i++) {
		file << "\n=========================\n";
		file << "Component " << i << "\n";
		file << *components[i] << "\n\n";
		file << "=========================\n";
	}
	
	return file; 
} // txtWrite()

//------------------ M-Projection

uniqptr<Factor> CanonicalGaussianMixture::momentMatch() const {
	unsigned M = comps_.size();
	ASSERT( M != 0, "There must be at least one mixand" );

	if (M == 1) return uniqptr<Factor>( comps_[0]->copy() );

	// Scope and dimension
	unsigned dimension = vars_.size();
	emdw::RVIds newVars(dimension);
	for(unsigned i = 0; i < dimension; i++) newVars[i] = vars_[i];

	// Determine the GM's total mass
	double totalMass = getLogMass();

	// First and second central moments
	ColVector<double> mean(dimension); mean.assignToAll(0.0);
	Matrix<double> cov = gLinear::zeros<double>(dimension, dimension);

	// Get the non-vacuous Gaussians' weights, means and covariances - Not very efficient, but whatever.
	for (unsigned i = 0; i < M; i++) {
		rcptr<Factor> factor = uniqptr<Factor>( comps_[i]->copy()  );
		rcptr<GaussCanonical> gc = std::dynamic_pointer_cast<GaussCanonical>(factor);

		// Determine relative weight
		double weight = exp( (gc->getLogMass()) - totalMass );

		//std::cout << "momentMatch(), weight : " << weight << std::endl;

		// Get the mean and covariance
		ColVector<double> mu = 1.0*(gc->getMean());
		Matrix<double> S = 1.0*(gc->getCov());

		mean += (weight)*(mu);
		cov += (weight)*( S + (mu)*(mu.transpose()) );
	} // for
	cov -= (mean)*(mean.transpose());

	rcptr<Factor> matched = uniqptr<Factor>(new GaussCanonical(newVars, mean, cov));
	std::dynamic_pointer_cast<GaussCanonical>(matched)->adjustLogMass(totalMass);

	return uniqptr<Factor>( matched->copy() );
} // momentMatch()

uniqptr<Factor> CanonicalGaussianMixture::momentMatchCGM() const {
	rcptr<Factor> matched = momentMatch();
	return uniqptr<Factor> (new CanonicalGaussianMixture(matched->getVars(), {matched}, true));
} // momentMatchCGM()

void CanonicalGaussianMixture::pruneAndMerge() {
	if (N_ > maxComp_) {
		std::vector<rcptr<Factor>> reduced = pruneComponents(comps_, maxComp_, threshold_, false);
		std::vector<rcptr<Factor>> merged =  mergeComponents(reduced, maxComp_, threshold_, unionDistance_);
		
		// Deep copy of vars
		emdw::RVIds vars(vars_.size());
		for (unsigned i = 0; i < vars_.size(); i++) vars[i] = vars_[i];

		classSpecificConfigure( vars, 
					merged, 
					true,
					maxComp_,
					threshold_,
					unionDistance_,
					inplaceNormalizer_,
					normalizer_,
					inplaceAbsorber_,
					absorber_,
					inplaceCanceller_,
					canceller_,
					marginalizer_,
					observeAndReducer_,
					inplaceDamper_);
	} // if
} //pruneAndMerge()

//---------------- Adjust Mass

void CanonicalGaussianMixture::adjustMass(const double mass) {
	for (rcptr<Factor> c : comps_) std::dynamic_pointer_cast<GaussCanonical>(c)->adjustMass(mass);
} // adjustMass()

//---------------- Useful get methods

std::vector<rcptr<Factor>> CanonicalGaussianMixture::getComponents() const { 
	std::vector<rcptr<Factor>> components(comps_.size());

	for (unsigned i = 0; i < comps_.size(); i++) components[i] = uniqptr<Factor>( comps_[i]->copy() );

	return components; 
} // getComponents()

double CanonicalGaussianMixture::getNumberOfComponents() const { return N_; } // getNumberOfComponents()

double CanonicalGaussianMixture::getMass() const {
	double mass = getLogMass();
	if (std::isinf(mass)) return 0;
	return exp(getLogMass());
} // getMass()

double CanonicalGaussianMixture::getLogMass() const {
	std::vector<double> masses;
	
	// Get the compnents logarithmic mass - ignoring all components with zero linear mass
	for (unsigned i = 0; i < N_; i++) {
		double mass= (std::dynamic_pointer_cast<GaussCanonical>(comps_[i]))->getLogMass();
		if (!std::isinf(mass)) masses.push_back(mass);
	} // for

	// If the mixture has no mass - return 0
	unsigned M = masses.size();
	if (M == 0) return -std::numeric_limits<double>::infinity();
	
	// Else work out the mass
	double maxMass= *std::max_element(masses.begin(), masses.end());
	double linearSum = 0;

	for (unsigned i = 0; i < M; i++) linearSum += exp(masses[i] - maxMass);
	return maxMass + log(linearSum);
} // getMass()


std::vector<double> CanonicalGaussianMixture::getWeights() const {
	std::vector<double> weights(comps_.size());
	for (unsigned i = 0; i < comps_.size(); i++) {
		weights[i]  = (std::dynamic_pointer_cast<GaussCanonical>(comps_[i]))->getMass() ;
	} // for
	return weights;
} // getWeights()

std::vector<ColVector<double>> CanonicalGaussianMixture::getMeans() const {
	std::vector<ColVector<double>> means;
	for (rcptr<Factor> c : comps_) means.push_back( (std::dynamic_pointer_cast<GaussCanonical>(c))->getMean() );
	return means;
} // getMeans()

std::vector<Matrix<double>> CanonicalGaussianMixture::getCovs() const {
	std::vector<Matrix<double>> covs;
	for (rcptr<Factor> c : comps_) covs.push_back( (std::dynamic_pointer_cast<GaussCanonical>(c))->getCov() );
	return covs;
} // getCovs()

std::vector<double> CanonicalGaussianMixture::getG() const {
	std::vector<double> g;
	for (rcptr<Factor> c : comps_) g.push_back( (std::dynamic_pointer_cast<GaussCanonical>(c))->getG() );
	return g;
} // getG()

std::vector<ColVector<double>> CanonicalGaussianMixture::getH() const {
	std::vector<ColVector<double>> info;
	for (rcptr<Factor> c : comps_) info.push_back( (std::dynamic_pointer_cast<GaussCanonical>(c))->getH() );
	return info;
} // getH()

std::vector<Matrix<double>> CanonicalGaussianMixture::getK() const {
	std::vector<Matrix<double>> prec;
	for (rcptr<Factor> c : comps_) prec.push_back( (std::dynamic_pointer_cast<GaussCanonical>(c))->getK());
	return prec;
} // getK()

//==================================================FactorOperators======================================

//------------------Family 1: Normalization
//
const std::string& InplaceNormalizeCGM::isA() const {
	static const std::string CLASSNAME("InplaceNormalizeCGM");
	return CLASSNAME;
} // isA()

void InplaceNormalizeCGM::inplaceProcess(CanonicalGaussianMixture* lhsPtr) {
	CanonicalGaussianMixture& lhs(*lhsPtr);
	std::vector<rcptr<Factor>> lhsComp = lhs.getComponents();

	// Get the total mass
	double totalMass = lhs.getLogMass();

	// Divide through by the total mass
	for (rcptr<Factor> c : lhsComp) std::dynamic_pointer_cast<GaussCanonical>(c)->adjustLogMass(-totalMass);

	// Reconfigure
	lhs.classSpecificConfigure(lhs.getVars(), 
			lhsComp, 
			true, 
			lhs.maxComp_,
			lhs.threshold_,
			lhs.unionDistance_,
			lhs.inplaceNormalizer_,
			lhs.normalizer_,
			lhs.inplaceAbsorber_,
			lhs.absorber_,
			lhs.inplaceCanceller_,
			lhs.canceller_,
			lhs.marginalizer_,
			lhs.observeAndReducer_,
			lhs.inplaceDamper_);
} // inplaceProcess()

const std::string& NormalizeCGM::isA() const {
	static const std::string CLASSNAME("NormalizeCGM");
	return CLASSNAME;
} // isA()

Factor* NormalizeCGM::process(const CanonicalGaussianMixture* lhsPtr) {
	CanonicalGaussianMixture* fPtr = new CanonicalGaussianMixture(*lhsPtr);
	InplaceNormalizeCGM ipNorm;
	
	try { 
		ipNorm.inplaceProcess(fPtr); 
	} catch (const char* s) {
		std::cout << __FILE__ << __LINE__ << " call to 'inplaceProcess' failed" << std::endl;
		throw s;
	}

	return fPtr;
} // process()


//------------------Family 2: Absorption, Cancellation

const std::string& InplaceAbsorbCGM::isA() const {
	static const std::string CLASSNAME("InplaceAbsorbCGM");
	return CLASSNAME;
} // isA()

void InplaceAbsorbCGM::inplaceProcess(CanonicalGaussianMixture* lhsPtr, const Factor* rhsFPtr) {
	// Try cast the pointer to CanonicalGaussianMixture 
	CanonicalGaussianMixture& lhs(*lhsPtr);
	const CanonicalGaussianMixture* rhsCGMPtr = dynamic_cast<const CanonicalGaussianMixture*>(rhsFPtr);
	
	// New components
	std::vector<rcptr<Factor>> product; 
	std::vector<rcptr<Factor>> lhsComps = lhs.getComponents();

	// If it isn't a CanonicalGaussianMixture then GaussCanonical should do all the validation.
	if (rhsCGMPtr){
		const CanonicalGaussianMixture& rhs(*rhsCGMPtr);
		std::vector<rcptr<Factor>> rhsComps = rhs.getComponents();

		for (rcptr<Factor> i : lhsComps) {
			for (rcptr<Factor> j : rhsComps) {
				product.push_back(i->absorb(j));
			}
		}
	} else { 
		const GaussCanonical* rhsGCFPtr = dynamic_cast<const GaussCanonical*>(rhsFPtr);
		const GaussCanonical& rhs(*rhsGCFPtr);
		
		product.resize(lhsComps.size());
		for (unsigned i = 0; i < lhsComps.size(); i++)  product[i] = lhsComps[i]->absorb(uniqptr<Factor>( rhs.copy()  ));
	}
	
	emdw::RVIds vars = product[0]->getVars();

	// Reconfigure the class
	lhs.classSpecificConfigure(vars, 
				product, 
				true,
				lhs.maxComp_,
				lhs.threshold_,
				lhs.unionDistance_,
				lhs.inplaceNormalizer_,
				lhs.normalizer_,
				lhs.inplaceAbsorber_,
				lhs.absorber_,
				lhs.inplaceCanceller_,
				lhs.canceller_,
				lhs.marginalizer_,
				lhs.observeAndReducer_,
				lhs.inplaceDamper_);
} // inplaceProcess()

const std::string& AbsorbCGM::isA() const {
	static const std::string CLASSNAME("AbsorbCGM");
	return CLASSNAME;
} // isA()

Factor* AbsorbCGM::process(const CanonicalGaussianMixture* lhsPtr, const Factor* rhsFPtr) {
	CanonicalGaussianMixture* fPtr = new CanonicalGaussianMixture(*lhsPtr);
	InplaceAbsorbCGM ipAbsorb;
	
	try { 
		ipAbsorb.inplaceProcess(fPtr, rhsFPtr); 
	} catch (const char* s) {
		std::cout << __FILE__ << __LINE__ << " call to 'inplaceProcess' failed" << std::endl;
		throw s;
	}

	return fPtr;
} // process()

const std::string& InplaceCancelCGM::isA() const {
	static const std::string CLASSNAME("InplaceCancelCGM");
	return CLASSNAME;
} // isA()

void InplaceCancelCGM::inplaceProcess(CanonicalGaussianMixture* lhsPtr, const Factor* rhsFPtr) {
	// Try cast the pointer to CanonicalGaussianMixture 
	CanonicalGaussianMixture& lhs(*lhsPtr);
	const CanonicalGaussianMixture* rhsCGMPtr = dynamic_cast<const CanonicalGaussianMixture*>(rhsFPtr);
	const CanonicalGaussianMixture& rhs(*rhsCGMPtr); // Not very safe
	
	// New components
	std::vector<rcptr<Factor>> lhsComps = lhs.getComponents();
	rcptr<Factor> single;

	//std::cout << "Dividend mixture: " << rhs << std::endl;
	
	// If it isn't a CanonicalGaussianMixture then GaussCanonical should do all the validation.
	if (rhsCGMPtr) single = rhs.momentMatch();
	else single = uniqptr<Factor>(rhsFPtr->copy());

	//std::cout << "Dividend approximation: " << *single << std::endl;

	// Divide through by a single GaussCanonical
	std::vector<rcptr<Factor>> quotient; quotient.clear();
	for (rcptr<Factor> i : lhsComps) { 
		quotient.push_back( i->cancel(single)  ); 
	} // for

	// Reconfigure the class
	lhs.classSpecificConfigure(quotient[0]->getVars(), quotient, true,
				lhs.maxComp_,
				lhs.threshold_,
				lhs.unionDistance_,
				lhs.inplaceNormalizer_,
				lhs.normalizer_,
				lhs.inplaceAbsorber_,
				lhs.absorber_,
				lhs.inplaceCanceller_,
				lhs.canceller_,
				lhs.marginalizer_,
				lhs.observeAndReducer_,
				lhs.inplaceDamper_);
} // inplaceCancel()

const std::string& CancelCGM::isA() const {
	static const std::string CLASSNAME("CancelCGM");
	return CLASSNAME;
} // isA()


Factor* CancelCGM::process(const CanonicalGaussianMixture* lhsPtr, const Factor* rhsFPtr) {
	CanonicalGaussianMixture* fPtr = new CanonicalGaussianMixture(*lhsPtr);
	InplaceCancelCGM ipCancel;
	
	try { 
		ipCancel.inplaceProcess(fPtr, rhsFPtr); 
	} catch (const char* s) {
		std::cout << __FILE__ << __LINE__ << " call to 'inplaceProcess' failed" << std::endl;
		throw s;
	}

	return fPtr;
} // process()


//------------------Family 3: Marginalization

const std::string& MarginalizeCGM::isA() const {
	static const std::string CLASSNAME("MarginalizeCGM");
	return CLASSNAME;
} // isA()

Factor* MarginalizeCGM::process(const CanonicalGaussianMixture* lhsPtr, const emdw::RVIds& variablesToKeep,
		bool presorted) {
	const CanonicalGaussianMixture& lhs(*lhsPtr);
	std::vector<rcptr<Factor>> lhsComps = lhs.getComponents();
	unsigned M = lhsComps.size();	
	std::vector<rcptr<Factor>> result(M);

	// If everything is marginalized out.
	if (!variablesToKeep.size()) return new CanonicalGaussianMixture(variablesToKeep, true);

	// Let GaussCanonical sort it all out for us.
	for (unsigned i = 0; i < M; i++) result[i] = (lhsComps[i])->marginalize(variablesToKeep, presorted);

	return new CanonicalGaussianMixture(result[0]->getVars(), 
				result, 
				true,
				lhs.maxComp_,
				lhs.threshold_,
				lhs.unionDistance_,
				lhs.inplaceNormalizer_,
				lhs.normalizer_,
				lhs.inplaceAbsorber_,
				lhs.absorber_,
				lhs.inplaceCanceller_,
				lhs.canceller_,
				lhs.marginalizer_,
				lhs.observeAndReducer_,
				lhs.inplaceDamper_ );
} // process()


//------------------Family 4: ObserveAndReduce

const std::string& ObserveAndReduceCGM::isA() const {
	static const std::string CLASSNAME("ObserveAndReduceCGM");
	return CLASSNAME;
} // isA()

Factor* ObserveAndReduceCGM::process(const CanonicalGaussianMixture* lhsPtr, const emdw::RVIds& variables,
		const emdw::RVVals& assignedVals, bool presorted) {
	const CanonicalGaussianMixture& lhs(*lhsPtr);
	std::vector<rcptr<Factor>> lhsComps = lhs.getComponents();
	std::vector<rcptr<Factor>> result; //result.clear();

	// If nothing was observed.
	if(!variables.size()) return lhs.copy(); 

	// Let GaussCanonical sort it all out for us.
	for (rcptr<Factor> c : lhsComps) result.push_back(c->observeAndReduce(variables, assignedVals, presorted));

	return new CanonicalGaussianMixture(result[0]->getVars(), 
			        result, 
				true,
				lhs.maxComp_,
				lhs.threshold_,
				lhs.unionDistance_,
				lhs.inplaceNormalizer_,
				lhs.normalizer_,
				lhs.inplaceAbsorber_,
				lhs.absorber_,
				lhs.inplaceCanceller_,
				lhs.canceller_,
				lhs.marginalizer_,
				lhs.observeAndReducer_,
				lhs.inplaceDamper_ );
} // process()


//------------------Family 5: Damping

const std::string& InplaceWeakDampingCGM::isA() const {
	static const std::string CLASSNAME("InplaceWeakDampingCGM");
	return CLASSNAME;
} // isA()

// TODO: Complete this!!!
double InplaceWeakDampingCGM::inplaceProcess(const CanonicalGaussianMixture* lhsPtr, const Factor* rhsPtr, double df) {
	return 0.0;
} // inplaceProcess()

//------------------ M-Projections

uniqptr<Factor> mProject(const std::vector<rcptr<Factor>>& components) {
	// Old means and covariances
	std::cout << "mProject()" << std::endl;
	unsigned M = components.size();
	ASSERT( M != 0, "There must be at least one mixand" );

	// The GM's components
	std::vector<double> w(M);
	std::vector<ColVector<double>> mu(M);
	std::vector<Matrix<double>> S(M);
	double totalMass = 0.0;

	// Scope and dimension
	emdw::RVIds vars = (components.back())->getVars();
	unsigned dimension = vars.size();

	// First and second central moments
	ColVector<double> mean(dimension); mean *= 0.0;
	Matrix<double> cov = gLinear::zeros<double>(dimension, dimension); cov *= 0.0;

	// Get the non-vacuous Gaussians' weights, means and covariances - Not very efficient, but whatever.
	for (unsigned i = 0; i < M; i++) {
		if (components[i]->noOfVars() != 0) { // Horrible hack to check if vacuous
			rcptr<Factor> factor = uniqptr<Factor>( components[i]->copy()  );
			rcptr<GaussCanonical> gc = std::dynamic_pointer_cast<GaussCanonical>(factor);

			// Get the mean and covariance
			mu[i] = gc->getMean();
			S[i] = gc->getCov();

			// Determine the weight
			w[i] = gc->getMass();
			totalMass += w[i];
		} else {
			w[i] = 0.0; 
			mu[i] = ColVector<double>(dimension); mu[i] *= 0;
			S[i] = gLinear::zeros<double>(dimension, dimension); S[i] *= 0;
		} // if
	} // for

	// Determine the first two moments of the mixture
	for (unsigned i = 0; i < M; i++) {
		double weight = w[i]/totalMass;
		mean += (weight)*(mu[i]);
		cov += (weight)*( S[i] + (mu[i])*(mu[i].transpose())  );
	} // for
	cov -= (mean)*(mean.transpose());

	std::cout << "After mProject()" << std::endl;

	return uniqptr<Factor>(new GaussCanonical(vars, mean, cov) );
} // mProject()

//------------------ Pruning and Merging

std::vector<rcptr<Factor>> pruneComponents(const std::vector<rcptr<Factor>>& components, const unsigned maxComp, const double threshold, bool clip)  {
	// Reduced components
	std::vector<rcptr<Factor>> reduced; reduced.clear();
	
	// Mass
	std::vector<double> originalMass; originalMass.clear();
	std::vector<double> reducedMass; reducedMass.clear();

	for (rcptr<Factor> c : components) {
		double mass = std::dynamic_pointer_cast<GaussCanonical>(c)->getLogMass();
		
		originalMass.push_back(mass);
		
		if (mass > threshold && ~std::isinf(mass) && ~std::isnan(mass)) {
			reduced.push_back( uniqptr<Factor>(c->copy()) );
			reducedMass.push_back(mass);
		} // if
	} // for

	// If everything should is below threshold - just take the largest components
	if (reduced.size() == 0) {
		// Sort the original weights

		std::vector<size_t> sortedIndices = sortIndices( originalMass, std::greater<double>() );
		std::vector<rcptr<Factor>> newComps = extract<rcptr<Factor>>(components, sortedIndices);

		// Only take the maxComp largest components
		std::vector<rcptr<Factor>> clipped; clipped.clear();
		for (unsigned i = 0; i < maxComp; i++) clipped.push_back( uniqptr<Factor> ( newComps[i]->copy() ) );

		// Return the reduced mixture
		return clipped;
	} // if

	// If this is you only reduction technique and you still have too many components
	if (clip && reduced.size() > maxComp) {
		// Sort the original weights
		std::vector<size_t> sortedIndices = sortIndices( reducedMass, std::greater<double>() );
		std::vector<rcptr<Factor>> newComps = extract<rcptr<Factor>>(reduced, sortedIndices);

		// Only take the maxComp largest components
		std::vector<rcptr<Factor>> clipped; clipped.clear();
		for (unsigned i = 0; i < maxComp; i++) clipped.push_back( uniqptr<Factor> ( newComps[i]->copy() ) );

		// Return the reduced mixture
		return clipped;
	} // if

	return reduced;
} // pruneComponents()

std::vector<rcptr<Factor>> mergeComponents(const std::vector<rcptr<Factor>>& components, const unsigned maxComp,
		const double threshold, const double unionDistance) {
	ASSERT( components.size() != 0, "There must be at least one mixand." );

	// Deep copy of vars
	emdw::RVIds oldVars = (components.back())->getVars();
	unsigned Q = oldVars.size();

	// Local variables
	std::vector<rcptr<Factor>> merged; merged.clear();
	std::vector<rcptr<Factor>> comps; comps.clear();
	std::vector<double> masses; masses.clear();

	// Get the mass and copy the factors
	for (rcptr<Factor> c : components) {
		rcptr<GaussCanonical> gc = std::dynamic_pointer_cast<GaussCanonical>(c);
		if (!std::isinf(gc->getLogMass())) {
			comps.push_back( uniqptr<Factor>( c->copy() ) );
			masses.push_back( gc->getLogMass() );
		}
	} // for	

	// If there is nothing of significant mass
	if (comps.size() == 0) return components;

	// Determine the total log mass of the mixture
	double maxMass= *std::max_element(masses.begin(), masses.end());
	double linearSum = 0;

	for (unsigned i = 0; i < masses.size(); i++) linearSum += exp(masses[i] - maxMass);
	double totalMass = maxMass + log(linearSum);

	// Sort the components according to mass
	std::vector<size_t> sortedIndices = sortIndices( masses, std::greater<double>() );
	std::vector<double> w = extract<double>( masses, sortedIndices );
	std::vector<rcptr<Factor>> newComps = extract<rcptr<Factor>>(comps, sortedIndices);

	// Merge closely spaced components
	while ( !newComps.empty() ) {
		// Dominant components mean
		ColVector<double> mu_0 = 1.0*std::dynamic_pointer_cast<GaussCanonical>(newComps[0])->getMean();
		unsigned L = w.size();

		// Local variables
		std::map<unsigned, bool> indices; 
		ColVector<double> mu(Q); mu.assignToAll(0.0);
		Matrix<double> S = gLinear::zeros<double>(Q, Q);
		double g = 0;

		// Create a merged super Gaussian
		for (unsigned i = 0; i < L; i++) {
			rcptr<GaussCanonical> gc = std::dynamic_pointer_cast<GaussCanonical>( newComps[i]  );
			if (gc->mahalanobis(mu_0) <= unionDistance) {
				ColVector<double> mean = 1.0*(gc->getMean());
				
				double weight = exp(w[i] - totalMass); // Relative mass
				g += weight; // Linear sum of weights
				mu += weight*(mean);
				S += weight*( gc->getCov() + (mean - mu_0)*( (mean - mu_0).transpose() ) );
				
				indices[i] = true;
			} // if
		} // for

		// Remove merged indices from the list -- Terrible method, but safe-ish.
		std::vector<rcptr<Factor>> cTemp; cTemp.clear();
		std::vector<double> wTemp; wTemp.clear();
		for (unsigned i = 0; i < L; i++) {
			if (!indices[i]) {
				cTemp.push_back(uniqptr<Factor> (newComps[i]->copy()) );
				wTemp.push_back(w[i]);
			} // if
		} // for

		// Reassign temporary variables to local variables
		newComps.clear(); w.clear();
		for (unsigned i = 0 ; i < cTemp.size(); i++) {
			newComps.push_back(uniqptr<Factor> (cTemp[i]->copy()) );
			w.push_back(wTemp[i]);
		} // for

		// Create a new Gaussian
		emdw::RVIds newVars(Q);
		for (unsigned i = 0; i < Q; i++) newVars[i] = oldVars[i];
		ColVector<double> newMean = mu/g; Matrix<double> newS = S/g;
		rcptr<Factor> scaled = uniqptr<Factor> (new GaussCanonical( newVars, newMean, newS, false ));

		// Determine new log mass
		double logMass = totalMass + log(g);
		std::dynamic_pointer_cast<GaussCanonical>(scaled)->adjustLogMass(logMass);
		
		// Push the merged component back
		merged.push_back( uniqptr<Factor>(scaled->copy()) );
	} // while	

	// If there are still too many components
	if (merged.size() > maxComp) {
		// Local variables
		std::vector<rcptr<Factor>> sigComps; sigComps.clear();
		std::vector<double> sigWeights; sigWeights.clear();
		
		// Extract weights and copy
		for ( rcptr<Factor> c : merged ) {
			sigComps.push_back( uniqptr<Factor> (c->copy() ) );
			sigWeights.push_back( std::dynamic_pointer_cast<GaussCanonical>(c)->getLogMass() );
		} // for
		
		// Sort according to weight
		std::vector<size_t> sorted = sortIndices( sigWeights, std::greater<double>() );
		std::vector<rcptr<Factor>> ordered = extract<rcptr<Factor>>(sigComps, sorted);
		
		// Select only the N largest components	
		merged.clear();
		for (unsigned i = 0; i < maxComp; i++) merged.push_back(ordered[i]);
	} // if

	return merged;
} // mergeComponents()

