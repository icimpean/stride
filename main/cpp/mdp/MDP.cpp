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
#include "pop/Person.h"
#include "pop/Population.h"
#include "sim/Sim.h"
#include "util/FileSys.h"
#include "util/RunConfigManager.h"
#include "util/TimeStamp.h"

#include <utility>

namespace stride {

using namespace std;
using namespace stride::util;
using namespace EventLogMode;
using namespace stride::ContactType;

MDP::MDP()
    : m_config(), m_simulator(nullptr), m_runner(nullptr), m_rnMan(), m_age_groups(),
      m_mRNA_properties(nullptr), m_adeno_properties(nullptr)
{
}

MDP::~MDP() {
    cout << "Deleting MDP attributes..." << endl;
    ClearSimulation();
}

void MDP::Create(const std::string& configPath,
                 std::shared_ptr<VaccineProperties> mRNA_properties, std::shared_ptr<VaccineProperties> adeno_properties,
                 int seed, const std::string& outputDir, const std::string& outputPrefix)
{
    boost::property_tree::ptree configPt = FileSys::ReadPtreeFile(configPath);
    MDP::Create_(configPt, seed, outputDir, outputPrefix);
    m_mRNA_properties = mRNA_properties;
    m_adeno_properties = adeno_properties;
}

void MDP::Create_(const boost::property_tree::ptree& config, int seed,
                  const std::string& outputDir, const std::string& outputPrefix)
{
    // Update the config
    m_config = config;
    // Update the output directory
    if (!outputDir.empty()) {
        m_config.put("run.output_prefix", outputDir);
    }
    // From SimController.Control
    // add timestamp if no output prefix specified
    else if (m_config.get<string>("run.output_prefix", "").empty()) {
        m_config.put("run.output_prefix", TimeStamp().ToTag().append("/"));
    }
    // Sort the configuration details
    m_config.sort();
    // Update the control helper since we didn't supply it at instantiation
    ControlHelper::m_config = m_config;
    ControlHelper::m_output_prefix = outputDir;

    // From SimController.Control
    // -----------------------------------------------------------------------------------------
    // Prelims.
    // -----------------------------------------------------------------------------------------
    CheckEnv();
    CheckOutputPrefix();
    InstallLogger();
    LogStartup();

    // Update the output directory to the prefix directory for current simulation
    if (!outputPrefix.empty()) {
        m_config.put("run.output_prefix", outputPrefix);
        // Update the control helper since we didn't supply it at instantiation
        ControlHelper::m_config = m_config;
        ControlHelper::m_output_prefix = outputPrefix;
    }

    if (seed != NULL) {
        m_stride_logger->info("Setting seed {}", seed);
        m_config.put("run.rng_seed", seed);
    }

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

    // -----------------------------------------------------------------------------------------
    // Vaccines: Create the age groups for vaccine sampling later
    // -----------------------------------------------------------------------------------------
    m_rnMan = rnMan;
    CreateAgeGroups();
}

void MDP::UpdateCntReduction(std::vector<double> workplace_distancing, std::vector<double> community_distancing,
                             std::vector<double> collectivity_distancing)
{
    m_simulator->GetCalendar()->UpdateCntReduction(workplace_distancing, community_distancing, collectivity_distancing);
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
    // Get the vaccine properties for the given vaccine type
    std::shared_ptr<VaccineProperties> vaccineProperties;
    // mRNA
    if (vaccineType == mRNA) { vaccineProperties = m_mRNA_properties; }
    // Adeno
    else if (vaccineType == adeno) { vaccineProperties = m_adeno_properties; }

    // Vaccinate the sampled people
    for (unsigned int id : sampled) {
        Person& p = pop->at(id);
        auto vaccine = vaccineProperties->GetVaccine();
        p.SetVaccine(vaccine);
        // Another person vaccinated from the age group
        m_vaccinated_age_groups[ageGroup] += 1;
    }
    // Log info
    m_stride_logger->info("[Vaccinate] {}/{} people vaccinated from age group {} with vaccine {}",
                          sampled.size(), availableVaccines, ageGroup, vaccineType);
}

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

std::map<AgeGroup, unsigned int> MDP::GetAgeGroupSizes()
{
    // Note: takes the number of people per age group that are not yet vaccinated
    //      => these values may change over time as people get vaccinated
    std::map<AgeGroup, unsigned int> groupCounts;
    for (AgeGroup ageGroup : AllAgeGroups) { groupCounts[ageGroup] = m_age_groups[ageGroup].size(); }
    return groupCounts;
}

unsigned int MDP::GetTotalInfected() const
{
    return m_simulator->GetPopulation()->GetTotalInfected();
}

unsigned int MDP::CountInfectedCases() const
{
    return m_simulator->GetPopulation()->CountInfectedCases();
}

unsigned int MDP::CountExposedCases() const
{
    return m_simulator->GetPopulation()->CountExposedCases();
}

unsigned int MDP::CountInfectiousCases() const
{
    return m_simulator->GetPopulation()->CountInfectiousCases();
}

unsigned int MDP::CountSymptomaticCases() const
{
    return m_simulator->GetPopulation()->CountSymptomaticCases();
}

unsigned int MDP::CountHospitalisedCases() const
{
    return m_simulator->GetPopulation()->CountHospitalisedCases();
}

unsigned int MDP::GetTotalHospitalised() const
{
    return m_simulator->GetPopulation()->GetTotalHospitalised();
}

unsigned int MDP::GetAtRisk() const
{
    return m_simulator->GetPopulation()->GetAtRisk();
}

void MDP::CreateAgeGroups()
{
    m_stride_logger->info("Creating age groups...");
    // Create an empty mapping for each age group
    for (AgeGroup ageGroup : AllAgeGroups) { m_age_groups[ageGroup] = std::vector<unsigned int>(); }
    for (AgeGroup ageGroup : AllAgeGroups) { m_vaccinated_age_groups[ageGroup] = 0; }
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
    std::vector<unsigned int> sampled;
    // Take the last elements of the shuffled age group and remove them from the vector
    while (!m_age_groups[ageGroup].empty() && sampled.size() < samples) {
        sampled.push_back(m_age_groups[ageGroup].back());
        m_age_groups[ageGroup].pop_back();
    }
    return sampled;
}

void MDP::ClearSimulation()
{
    // Clear the ContactPoolSys
    cout << "\tClearing ContactPoolSys..." << endl;
    m_simulator->GetPopulation()->RefPoolSys().ClearContactPools();

    cout << "\tClearing population..." << endl;
    m_simulator->GetPopulation()->clear();

    cout << "\tClearing simulator and runner..." << endl;
    m_runner.reset();
    m_simulator.reset();

    cout << "\tClearing age groups..." << endl;
    m_age_groups.clear();

    cout << "\tClearing vaccine properties..." << endl;
    m_mRNA_properties.reset();
    m_adeno_properties.reset();
}

} // namespace stride
