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
 * Helpers for age groups.
 */

#pragma once

namespace stride {

/// Age groups enumeration
/// TODO: refactor this to pop/Age.h?
enum AgeGroup {
    /// Age 0-4
    children,
    /// Age 5-18
    youngsters,
    /// Age 19-25
    young_adults,
    /// Age 26-64
    adults,
    /// Age 65+
    elderly
};


/// Get the age group for a given age
inline AgeGroup GetAgeGroup(unsigned int age) {
        /// Age 0-4
        if (age < 5) { return children; }
        /// Age 5-18
        else if (age < 19) { return youngsters; }
        /// Age 19-25
        else if (age < 26) { return young_adults; }
        /// Age 26-64
        else if (age < 65) { return adults; }
        /// Age 65+
        else { return elderly; }
}

} // namespace stride
