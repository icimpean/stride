/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2021
 */

/**
 * @file
 * Header for the MDP class.
 */

#include "contact/AgeContactProfiles.h"
#include "contact/EventLogMode.h"
#include "contact/InfectorExec.h"
#include "disease/PublicHealthAgency.h"
#include "disease/TransmissionProfile.h"
#include "disease/UniversalTesting.h"
#include "util/RnMan.h"
#include "util/RnHandler.h"
#include "execs/ControlHelper.h"

#include <boost/property_tree/ptree.hpp>
#include <string>

namespace stride {

class Calendar;
class Population;
class Sim;
class SimRunner;

/**
 * Markov Decision Process for the stride simulation.
 * Exposes the simulation to distribute vaccines to groups of the population.
 *
 * TODO: following SimRunner class of creating a simulation & population
 */
class MDP : protected ControlHelper
{
public:
        /// Constructor for empty MDP.
        explicit MDP();

        /// Create an MDP (and the underlying simulation) from a given configuration
        /// TODO: default value for configuration for python call, or config file
        void Create(const std::string& configPath);
        void Create_(const boost::property_tree::ptree& config);

        /// Simulate a single day in the simulation
        /// TODO: return state, reward, done (end simulation)
        void Simulate_Day();

        /// Vaccinate a given age group with the given vaccine type and number of available vaccines
        /// TODO: types ageGroup & VaccineType
        void Vaccinate(int availableVaccines, int ageGroup, int vaccineType);


        /// Methods mapped from Sim class
//        std::shared_ptr<Calendar> GetCalendar() const { return m_sim.GetCalendar(); }
//        std::string GetConfigValue(const std::string& attribute) const { return m_config.get<std::string>(attribute); }
//        std::shared_ptr<Population> GetPopulation() { return m_sim.GetPopulation(); }
//        double GetTransmissionProbability() const { return m_sim.GetTransmissionProbability(); }
//        const TransmissionProfile& RefTransmissionProfile() const { return m_sim.RefTransmissionProfile(); }

private:
        boost::property_tree::ptree m_config;                       ///< Configuration property tree
        std::shared_ptr<Sim> m_simulator;                           ///< The simulation
        std::shared_ptr<SimRunner> m_sim_runner;                    ///< The runner for the simulation
};

} // namespace stride
