// Reference to header file(s) needed
#include "../../../cpp/mdp/Vaccines.h"
// Some extra (external) header files of the project required for successfully building the pybind library
//

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;


void init_vaccine_types(py::module &m) {
    // VaccineType enumeration
    py::enum_<stride::VaccineType>(m, "VaccineType")
            .value("noVaccine", stride::VaccineType::noVaccine)
            .value("mRNA", stride::VaccineType::mRNA)
            .value("adeno", stride::VaccineType::adeno);
    // All vaccine types
    m.attr("AllVaccineTypes") = stride::AllVaccineTypes;
}
