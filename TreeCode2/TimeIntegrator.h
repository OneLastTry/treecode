/*
 * TimeIntegrator.h
 *
 *  Created on: 22 Dec 2011
 *      Author: stefans
 */

#ifndef TIMEINTEGRATOR_H_
#define TIMEINTEGRATOR_H_

#include <vector>
#include <string>
#include <fstream>
#include "Particle.h"
#include "Tree.h"
#include "Node.h"
#include "bounds/BoundaryConditions.h"
#include "pushers/pusher.h"
#include "potentials/Potential.h"
#include "output/ParticleTracker.h"

namespace treecode {

template <class Vec, class Mat>
class TimeIntegrator {
public:
	/**
	 * @class TimeIntegrator
	 * @brief Class for the time integration of particles.
	 *
	 * The purpose of this class is to tie together, with
	 * a supplied particle pusher, various
	 * objects, such as potential, tree and boundary conditions.
	 */

	/**
	 * @brief Instantiate new TimeIntegrator.
	 * @param particles		List of particles.
	 * @param tree			Tree.
	 * @param bounds		Boundary conditions.
	 * @param pusher		Particle pusher.
	 */
	TimeIntegrator(
			double timestep, double max_time,
			std::vector<Particle<Vec,Mat>*>& particles,
			Tree<Vec,Mat>& tree,
			BoundaryConditions<Vec,Mat>& bounds,
			pusher::Pusher<Vec,Mat>& pusher,
			const AcceptanceCriterion<Vec,Mat>& mac):
		dt_(timestep), max_time_(max_time),
		bounds_(bounds), particles_(particles), pusher_(pusher),
		tree_(tree), energies_out_(NULL), mac_(mac)
	{}

	/**
	 * @brief Kick off the integration.
	 * @param precision		Precision to supply to the potential.
	 * @param output_every	Output every nth step.
	 * @param pos_file		File to output particle positions to (or NULL).
	 * @param vel_file		File to output particle velocities to (or NULL).
	 * @param energy_file	File to output energies to (or NULL).
	 */
	void start(potentials::Precision precision, unsigned int output_every){
		long int num_steps = max_time_/dt_;
		for(long int i=0;i<num_steps;i++){
			if(i == 2526)
				std::cout << " ";

			std::pair<double, double> energies = pusher_.push_particles(particles_, tree_, bounds_, precision, mac_);
			bounds_.timestepOver();

			std::cout << "Timestep " << i << " of " << num_steps << " complete (" << ((float)i/(float)num_steps)*100 << "%)" << std::endl;
			if( (i%output_every) == 0){
				if(energies_out_ != NULL)
					(*energies_out_) << (i * dt_) << "\t" << energies.first << "\t" << energies.second << std::endl;

				typedef output::ParticleTracker<Vec,Mat> track;
				BOOST_FOREACH(track* t, particle_trackers_)
					t->output();
			}
		}
	}

	/**
	 * @brief Destructor.
	 */
	~TimeIntegrator() {
		delete energies_out_;
	}

	/**
	 * @brief Set file to output system energies to
	 * @param filename File name
	 */
	void setEnergyOutputFile(const char* filename){
		energies_out_ = new std::ofstream(filename);
	}

	/**
	 * @brief Add particle tracker.
	 * @param tracker Tracker.
	 */
	void addParticleTracker(output::ParticleTracker<Vec,Mat>* tracker){
		particle_trackers_.push_back(tracker);
	}

private:
	double dt_, max_time_;
	BoundaryConditions<Vec,Mat>& bounds_;
	std::vector<Particle<Vec,Mat>*>& particles_;
	pusher::Pusher<Vec,Mat>& pusher_;
	Tree<Vec,Mat>& tree_;

	std::ofstream *energies_out_;
	const AcceptanceCriterion<Vec,Mat>& mac_;

	std::vector<output::ParticleTracker<Vec,Mat>* > particle_trackers_;
};

} /* namespace treecode */
#endif /* TIMEINTEGRATOR_H_ */
