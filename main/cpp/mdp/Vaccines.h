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
 * Header for the mRNA and Adeno Vaccines.
 */

#pragma once

#include <vector>


namespace stride {

// Python does not need access to vaccine class and its methods,
//  only specifies which vaccine to generate for simplicity
/// Vaccines enumeration
enum VaccineType { noVaccine, mRNA, adeno };

/// All the vaccine types
static const std::vector<VaccineType> AllVaccineTypes = { noVaccine, mRNA, adeno };

} // namespace stride
