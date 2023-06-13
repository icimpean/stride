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
 *  Copyright 2020, Libin P, Willem L
 */

/**
 * @file
 * Implementation for the HospitalisationConfig class.
 */

#include "HospitalisationConfig.h"

namespace stride {

void HospitalisationConfig::NoHospitalisationInit()
{
    for (unsigned int i=0; i <= MaximumAge(); ++i) {
        m_probabilities[i] = 0.0;
        m_delays[i] = 0.0;
    }
}

HospitalisationConfig::HospitalisationConfig() : m_probabilities(), m_delays()
{
    NoHospitalisationInit();
}

HospitalisationConfig::HospitalisationConfig(
        std::vector<unsigned int> ageCategories,
        std::vector<double> probabilities, std::vector<double> delays, double probability_factor)
        : m_probabilities(), m_delays()
{
    //default initialisation
    NoHospitalisationInit();

    //parse the age category probabilities/delays
    if (!ageCategories.empty()) {
        for (unsigned int i=0; i <= ageCategories.size() - 1; ++i) {
            unsigned int max = MaximumAge();
            if (i < ageCategories.size() - 1)
                max = ageCategories[i+1] - 1;
            for (unsigned int j=ageCategories[i]; j <= max; ++j) {
                double new_prob = probabilities[i] * probability_factor;
                if (new_prob > 1) { new_prob = 1; }
                m_probabilities[j] = new_prob;
                m_delays[j] = delays[i];
            }
        }
    }
}

}