// Reference to header file(s) needed
#include "../../../cpp/mdp/MDP.h"

// Some extra (external) header files of the project required for successfully building the pybind library
#include "../../../cpp/calendar/Calendar.h"
#include "../../../cpp/pop/Population.h"
#include "../../../cpp/sim/Sim.h"
#include "../../../cpp/sim/SimRunner.h"
#include <boost/property_tree/ptree.hpp>


#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;


void init_mdp(py::module &m) {
    // MDP class
    py::class_<stride::MDP, std::shared_ptr<stride::MDP>>(m, "MDP",
            "The Markov Decision Process wrapper around the STRIDE simulator")
            // Constructor
            .def(py::init<>())
            // Methods
            .def("Create", &stride::MDP::Create, py::arg("configPath"),
                 "Create a simulation from the given configuration file (.xml)")
            .def("Simulate_Day", &stride::MDP::Simulate_Day,
                 "Runs the simulator for a day")
            .def("Vaccinate", &stride::MDP::Vaccinate,
                 py::arg("availableVaccines"), py::arg("ageGroup"), py::arg("vaccineType"),
                 "Vaccinate a given age group with the given vaccine type for the available number of vaccines");
}
