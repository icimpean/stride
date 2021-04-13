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

#include "calendar/Calendar.h"
#include "contact/ContactType.h"
#include "contact/InfectorExec.h"
#include "disease/DiseaseSeeder.h"
#include "pop/Population.h"
#include "sim/Sim.h"
#include "sim/SimBuilder.h"
#include "sim/SimRunner.h"
#include "util/FileSys.h"
#include "util/RunConfigManager.h"
#include "util/TimeStamp.h"

#include <utility>

namespace stride {

using namespace std;
using namespace stride::util;
using namespace EventLogMode;

MDP::MDP()
    : m_config(), m_simulator(nullptr), m_sim_runner(nullptr)
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
    auto runner = make_shared<SimRunner>(m_config, m_simulator);
    RegisterViewers(runner);
    m_sim_runner = runner;
}

void MDP::Simulate_Day()
{
    // Run the simulation for a day
    m_sim_runner->Run(1);
}


void MDP::Vaccinate(int availableVaccines, int ageGroup, int vaccineType) {
    // TODO
    // Temporary print to indicate function is called (but does nothing to the simulation yet)
    cout << "[Vaccine]" << availableVaccines << " Vaccines of type" << vaccineType << " are distributed over age group " << ageGroup << endl;
}

} // namespace stride
