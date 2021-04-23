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
    : m_config(), m_simulator(nullptr), m_runner(nullptr), m_rnMan(), seed(), m_age_groups()
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
    RnMan        rnMan{info};  // TODO: change seed each new Create call

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

    // -----------------------------------------------------------------------------------------
    // Vaccines: Create the age groups for vaccine sampling later
    // -----------------------------------------------------------------------------------------
    m_rnMan = rnMan;
    CreateAgeGroups();
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

unsigned int MDP::SimulateDay()
{
    // Run the simulation for a day
    return MDP::Simulate(1);
}

unsigned int MDP::SimulateVaccinate(unsigned int numDays, unsigned int availableVaccines,
                                    AgeGroup ageGroup, VaccineType vaccineType)
{
    // Run the simulation the given number of days
    for (unsigned int i = 0; i < numDays; i++) {
        // Vaccinate
        Vaccinate(availableVaccines, ageGroup, vaccineType);
        // Simulate
        m_runner->Step();
    }
    // Return the number of infected
    return m_simulator->GetPopulation()->GetTotalInfected();
}

void MDP::Vaccinate(unsigned int availableVaccines, AgeGroup ageGroup, VaccineType vaccineType)
{
    // Get people availableVaccines people within the given age group
    std::shared_ptr<Population> pop = m_simulator->GetPopulation();
    // Sample from the given age group (none of these are vaccinated yet)
    std::vector<unsigned int> sampled = SampleAgeGroup(ageGroup, availableVaccines);

    // Vaccinate the sampled people
    for (unsigned int id : sampled) {
        Person& p = pop->at(id);
        auto vaccine = GetVaccine(vaccineType);
        p.SetVaccine(vaccine);
    }
    // Log info
    m_stride_logger->info("[Vaccinate] {}/{} people vaccinated from age group {} with vaccine {}",
                          sampled.size(), availableVaccines, ageGroup, vaccineType);
}

// TODO: old draft of Vaccinate, to be removed
//void MDP::Vaccinate(unsigned int availableVaccines, AgeGroup ageGroup, VaccineType vaccineType)
//{
//    // Get people availableVaccines people within the given age group
//    std::shared_ptr<Population> pop = m_simulator->GetPopulation();
//
//    // Simple loop for entire population
//    unsigned int persons_found{0U};
//    for (Person& p : *pop) {
//        // If the person is in the right age group AND not yet vaccinated
//        if (GetAgeGroup(p.GetAge()) == ageGroup && !p.IsVaccinated()) {
//            // Simple vaccine immunity // TODO: use the given vaccine type (currently based on ImmunitySeeder vaccination)
//            shared_ptr<ConstantVaccine::Properties> properties(new ConstantVaccine::Properties{"immunity", 1.0,1.0,1.0});
//            auto vaccine = std::unique_ptr<Vaccine>(new ConstantVaccine(properties));
//            p.SetVaccine(vaccine);
//            // Another person has been found to vaccinate
//            persons_found++;
// //            cout << "[Vaccine " << persons_found << "/" << availableVaccines << "] Person " << p.GetId() << " from age group " << ageGroup << " with age " << p.GetAge() << endl;
//            // If availableVaccines people have been vaccinated, stop iterating
//            if (persons_found == availableVaccines) { break; }
//        }
//    }
//    cout << "[Vaccine] " << persons_found << "/" << availableVaccines << " People vaccinated from age group " << ageGroup << endl;
//}

void MDP::End()
{
    m_runner->End();
}

unsigned int MDP::GetNumberOfDays()
{
    const auto numDays = m_config.get<unsigned int>("run.num_days");
    return numDays;
}

unsigned int MDP::GetPopulationSize()
{
    return m_simulator->GetPopulation()->size();
}

void MDP::CreateAgeGroups()
{
    m_stride_logger->info("Creating age groups...");
    // Create an empty mapping for each age group
    for (AgeGroup ageGroup : AllAgeGroups) { m_age_groups[ageGroup] = std::vector<unsigned int>(); }
    // Iterate over the population and add people to their age group
    std::shared_ptr<Population> pop = m_simulator->GetPopulation();
    for (Person& p : *pop) { m_age_groups[GetAgeGroup(p.GetAge())].push_back(p.GetId()); }
    // Remove unused capacity from the vectors and shuffle the values
    for (AgeGroup ageGroup : AllAgeGroups) {
        m_age_groups[ageGroup].shrink_to_fit();
        m_rnMan.Shuffle(m_age_groups[ageGroup], 0U);
    }
}

std::vector<unsigned int> MDP::SampleAgeGroup(AgeGroup ageGroup, unsigned int samples)
{
    // Get the age group to sample from
    std::vector<unsigned int> group = m_age_groups[ageGroup];
    std::vector<unsigned int> sampled;
    // Take the last elements of the shuffled age group and remove them from the vector
    while (!group.empty() && sampled.size() < samples) {
        sampled.push_back(group.back());
        group.pop_back();
    }
    return sampled;
}

// Helper function
inline std::unique_ptr<Vaccine> GetVaccine(VaccineType vaccineType) {
    // TODO: use LinearVaccine class instead?
    shared_ptr<ConstantVaccine::Properties> properties;
    // mRNA
    if (vaccineType == mRNA) {
        properties =
        shared_ptr<ConstantVaccine::Properties>(
                new ConstantVaccine::Properties{"mRNA vaccine", 0.95, 0.95, 1.00});
    }
    // Adeno
    else if (vaccineType == adeno) {
        properties =
                shared_ptr<ConstantVaccine::Properties>(
                new ConstantVaccine::Properties{"Adeno vaccine", 0.86, 0.86, 1.00});
    }
    // Else no vaccine (python does not supply noVaccine as an option so this should never happen)
    else {}
    // Create and return the vaccine
    return std::unique_ptr<ConstantVaccine>(new ConstantVaccine(properties));
}

} // namespace stride
