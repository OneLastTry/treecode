/*
 * CoulombForce.h
 *
 *  Created on: 6 Dec 2011
 *      Author: stefans
 */

#ifndef COULOMBFORCE_H_
#define COULOMBFORCE_H_

#include "Potential.h"
#include "../Particle.h"
#include "../Node.h"
#include "../bounds/BoundaryConditions.h"

namespace treecode {

namespace potentials {

template <class Vec, class Mat>
class CoulombForceThreeD : public Potential<Vec,Mat>{
//friend class InterpolatedEwaldSum;

public:

/**
 * @class CoulombForceThreeD "potentials/CoulombForce.h"
 * @brief Potential class representing an ``r-squared'' coulomb force.
 */

/**
 * @brief Construct a new CoulombForce object.
 * @param conf	Global configuration.
 * @param bc	Boundary conditions.
 */
CoulombForceThreeD(const treecode::Configuration<Vec>& conf, const treecode::BoundaryConditions<Vec,Mat>& bc) :
		configuration(conf), boundary(bc){}

/**
 * @brief Get force on a particle generated by a Node.
 *
 * The monopole force is simply: @f[ \vec{F}_m = \frac{q_p q_n \vec{r}}{r^3}, @f]
 * where @f$ \vec{r} @f$ is the displacement vector between the particle and the
 * node's centre of charge, @f$ q_p @f$ is the particle's charge and @f$ q_n @f$ is
 * the node's charge.
 *
 * The dipole force is given by:
 * @f[
 * \vec{F}_d = -\frac{1}{r^3} \mathbf{D}_n + \frac{3}{r^5} \vec{r} \, \vec{r}^T \mathbf{D}_n,
 * @f]
 * where @f$ \mathbf{D}_n @f$ is the node's dipole moments.
 *
 * The quadrupole force is given by:
 * @f[
 * \vec{F}_q = \frac{15}{2 r^7} (\vec{r}\cdot (\mathbf{Q} \vec{r})) \vec{r}
 * - \frac{3}{r^5} \mathbf{Q}_n \vec{r}
 * - \frac{3}{2 r^5} \mathrm{Tr}(\mathbf{Q}_n) \vec{r},
 * @f]
 * where @f$ \mathbf{Q}_n @f$ is the <em>intrinsic</em> quadrupole of the node.
 *
 * @param part		Particle to calculate the force at.
 * @param node		Node interacting with particle.
 * @param precision	Precision used (monopole, dipole, quadrupole).
 * @return	Force due to node on part.
 */
virtual Vec getForce(const Particle<Vec,Mat>& part, const Node<Vec,Mat>& node, Precision precision) const{
	Vec force = Vec::Zero();
	Vec disp_vec = boundary.getDisplacementVector(part.getPosition(), node.getCentreOfCharge());
	double r  = 1.0/disp_vec.norm();
	//If we are doing a straight particle-particle interaction, apply force softening
//	if(node.getStatus() == Node::tree_status::LEAF)
		r  = 1.0/sqrt(disp_vec.squaredNorm() + configuration.getForceSoftening()*configuration.getForceSoftening());
	double r2 = r*r;
	double r3 = r2*r;

	//Monopole moment
	force += node.getCharge() * disp_vec * r3;

	//Dipole moment
	if(precision == dipole || precision == quadrupole){
		double r5 = r3*r2;
		//Add diagonal contribution
		force -= r3 * node.getDipoleMoments();
		//Add off-diagonal contribution
		force += (disp_vec * disp_vec.transpose()) * node.getDipoleMoments() * 3 * r5;

		if(precision == quadrupole){
			double r7 = r5*r2;
			//r^-7 part:
			force += r7 * 15.0/2 * disp_vec.dot(node.getQuadrupoleMoments() * disp_vec) * disp_vec;
			//r^-5 part:
			force -= r5 * 3.0 * (node.getQuadrupoleMoments() * disp_vec);
			//We're using the intrisic quadrupole, not the traceless:
			force -= r5 * 3.0/2 * node.getQuadrupoleMoments().trace() * disp_vec;
		}

	}
	return part.getCharge() * force / (3.0 * configuration.getDensity());
}

/**
 * @brief Get electric potential caused by a node at the position of a particle.
 *
 * The monopole contribution is simply @f[\phi_m = \frac{q_n}{r} @f], where
 * definitions are the same as for CoulombForce::getForce().
 *
 * The dipole contribution is:
 * @f[ \phi_d = \frac{\vec{r} \cdot \mathbf{D}_n}{r^3}. @f]
 *
 * The quadrupole contribution is:
 * @f[ \phi_q = \frac{3}{2 r^5} \vec{r} \cdot (\mathbf{Q}_n \vec{r})
 * - \frac{1}{2 r^3} \mathrm{Tr}(\mathbf{Q}_n). @f]
 *
 *
 * @param part		Particle at which to calculate potential.
 * @param node		Node generating potential.
 * @param precision	Precision.
 * @return	Electric potential caused by node at part's position.
 */
virtual double getPotential(const Particle<Vec,Mat>& part, const Node<Vec,Mat>& node, Precision precision) const{
	double potential = 0;
	Vec disp_vec = boundary.getDisplacementVector(part.getPosition(), node.getCentreOfCharge());
	double r  = 1.0/disp_vec.norm();
	//If we are doing a straight particle-particle interaction, apply force softening
//	if(node.getStatus() == Node::tree_status::LEAF)
		r  = 1.0/sqrt(disp_vec.squaredNorm() + configuration.getForceSoftening()*configuration.getForceSoftening());
	double r2 = r*r;

	potential += node.getCharge() * r;

	if(precision == dipole || precision == quadrupole){
		double r3 = r2*r;
		potential += r3 * disp_vec.dot(node.getDipoleMoments());
		if(precision == quadrupole){
			double r5 = r3 * r2;
			potential += 3.0/2 * r5 * disp_vec.dot(node.getQuadrupoleMoments() * disp_vec);
			potential -= 0.5 * r3 * node.getQuadrupoleMoments().trace();
		}
	}
	return potential / (3.0 * configuration.getDensity());
}

/**
 * Destructor. Does nothing.
 */
virtual ~CoulombForceThreeD() {}

private:
	const treecode::Configuration<Vec>& configuration;
	const treecode::BoundaryConditions<Vec,Mat>& boundary;
};

} /* namespace potentials */
} /* namespace treecode */
#endif /* COULOMBFORCE_H_ */
