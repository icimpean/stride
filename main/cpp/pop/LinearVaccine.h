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
 * Header for the LinearVaccine class.
 */

#pragma once

#include <string>
#include <memory>

namespace stride {


class LinearVaccine : public Vaccine 
{
public:
        struct Properties {
            std::string m_id;
            double m_ve_susceptible;
            double m_ve_infectiousness;
            double m_ve_severe;
            unsigned short m_max_ve_day;            
        };

        ///
        explicit LinearVaccine(std::shared_ptr<Properties> properties) {
            m_properties = properties;
        }

        ~LinearVaccine() {}

private:
        double Ve(double ve) const {
            unsigned short x = m_counter;
            if (x >= m_properties->m_max_ve_day)
                return ve;
            else
                return x * (ve/m_properties->m_max_ve_day);
        }

public:
        ///
        virtual double GetVeSusceptible() const {
            return Ve(m_properties->m_ve_susceptible);
        }

        ///
        virtual double GetVeInfectiousness() const { 
            return Ve(m_properties->m_ve_infectiousness);
        }

        ///
        virtual double GetVeSevere() const { 
            return Ve(m_properties->m_ve_severe);
        }

private:
        std::shared_ptr<Properties> m_properties;
};

} // namespace stride
