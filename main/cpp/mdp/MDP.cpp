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
 * Implementation for the MDP class.
 */

#include "MDP.h"

#include "contact/ContactType.h"
#include "mdp/AgeGroup.h"
#include "mdp/MDPRunner.h"
#include "pop/ConstantVaccine.h"
#include "pop/Person.h"
#include "pop/Population.h"
#include "sim/Sim.h"
#include "sim/SimBuilder.h"
#include "util/FileSys.h"
#include "util/RunConfigManager.h"
#include "util/TimeStamp.h"

#include <utility>

namespace stride {

using namespace std;
using namespace stride::util;
using namespace EventLogMode;

MDP::MDP()
    : m_config(), m_simulator(nullptr), m_runner(nullptr)
{
}

void MDP::Create(const std::string& configPath) {
    boost::property_tree::ptree configPt = FileSys::ReadPtreeFile(configPath);
    MDP::Create_(configPt);
}

void MDP::Create_(const boost::property_tree::ptree& config) {
    // Update the config (also the control helper since we didn't supply it at instantiation)
    m_config = config;
    ControlHelper::m_config = m_config;
    // From SimController.Control
    // add timestamp if no output prefix specified
    if (m_config.get<string>("run.output_prefix", "").empty()) {
        m_config.put("run.output_prefix", TimeStamp().ToTag().append("/"));
    }
    // sort the configuration details
    m_config.sort();

    // From SimController.Control
    // -----------------------------------------------------------------------------------------
    // Prelims.
    // -----------------------------------------------------------------------------------------
    CheckEnv();
    CheckOutputPrefix();
    InstallLogger();
    LogStartup();

    // -----------------------------------------------------------------------------------------
    // Sim scenario: step 1, build a random number manager.
    // -----------------------------------------------------------------------------------------
    const RnInfo info{m_config.get<string>("run.rng_seed", "1,2,3,4"), "",
                      m_config.get<unsigned int>("run.num_threads")};
    RnMan        rnMan{info};

    // -----------------------------------------------------------------------------------------
    // Sim scenario: step 2, create a population, as described by the parameter in the config.
    // -----------------------------------------------------------------------------------------
    auto pop = Population::Create(m_config, m_stride_logger);

    // -----------------------------------------------------------------------------------------
    // Sim scenario: step 3, create a simulator, as described by the parameter in the config.
    // -----------------------------------------------------------------------------------------
    m_simulator = Sim::Create(m_config, pop, rnMan);

    // -----------------------------------------------------------------------------------------
    // Sim scenario: step , build a runner, register viewers.
    // -----------------------------------------------------------------------------------------
    auto runner = make_shared<MDPRunner>(m_config, m_simulator);
    RegisterViewers(runner);
    m_runner = runner;
}

unsigned int MDP::GetNumberOfDays()
{
    const auto numDays = m_config.get<unsigned int>("run.num_days");
    return numDays;
}

unsigned int MDP::Simulate(unsigned int numDays)
{
    // Run the simulation the given number of days
    for (unsigned int i = 0; i < numDays; i++) {
        m_runner->Step();
    }
    // Return the number of infected
    return m_simulator->GetPopulation()->GetTotalInfected();
}

unsigned int MDP::Simulate_Day()
{
    // Run the simulation for a day
    return MDP::Simulate(1);
}


void MDP::Vaccinate(unsigned int availableVaccines, AgeGroup ageGroup, int vaccineType) {
    // Vaccinate the given age group
    // TODO: use given vaccine type
    VaccinateAgeGroup(availableVaccines, ageGroup);
}

void MDP::VaccinateAgeGroup(unsigned int availableVaccines, AgeGroup ageGroup) {

    // Get people availableVaccines people within the given age group
    std::shared_ptr<Population> pop = m_simulator->GetPopulation();

    // Simple loop for entire population
    // TODO: create indices per age group to index, & remove vaccinated persons from indices?
    //  => Don't iterate over all persons (or all age groups) & sample from those not yet vaccinated
    // TODO: What happens when no people remain to be vaccinated in a certain group
    //  => return the number of vaccines remaining and repurpose or are they lost?
    unsigned int persons_found{0U};
    for (Person& p : *pop) {
        // If the person is in the right age group AND not yet vaccinated
        if (GetAgeGroup(p.GetAge()) == ageGroup && !p.IsVaccinated()) {
            // TODO: use the given vaccine type (currently based on ImmunitySeeder vaccination)
            //Simple vaccine immunity
            shared_ptr<ConstantVaccine::Properties> properties(new ConstantVaccine::Properties{"immunity", 1.0,1.0,1.0});
            auto vaccine = std::unique_ptr<Vaccine>(new ConstantVaccine(properties));
            p.SetVaccine(vaccine);
            // Another person has been found to vaccinate
            persons_found++;
//            cout << "[Vaccine " << persons_found << "/" << availableVaccines << "] Person " << p.GetId() << " from age group " << ageGroup << " with age " << p.GetAge() << endl;
            // If availableVaccines people have been vaccinated, stop iterating
            if (persons_found == availableVaccines) { break; }
        }
    }
    cout << "[Vaccine] " << persons_found << "/" << availableVaccines << " People vaccinated from age group " << ageGroup << endl;
}

void MDP::End()
{
    m_runner->End();
}

} // namespace stride
