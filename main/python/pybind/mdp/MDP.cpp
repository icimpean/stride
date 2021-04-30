// Reference to header file(s) needed
#include "../../../cpp/mdp/MDP.h"

// Some extra (external) header files of the project required for successfully building the pybind library
//


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
            .def("Create", &stride::MDP::Create,
                 py::arg("configPath"), py::arg("seed") = 0, py::arg("outputDir") = "", py::arg("outputPrefix") = "",
                 "Create a simulation from the given configuration file (.xml) "
                 "and optional output directory and prefix for the logs")
            .def("GetNumberOfDays", &stride::MDP::GetNumberOfDays,
                 "Get the number of days specified to run the simulator for")
            .def("GetPopulationSize", &stride::MDP::GetPopulationSize,
                 "Get the population size")
            .def("GetTotalInfected", &stride::MDP::GetTotalInfected,
                 "Get the cumulative number of cases")
            .def("CountInfectedCases", &stride::MDP::CountInfectedCases,
                 "Get the current number of infected cases")
            .def("CountExposedCases", &stride::MDP::CountExposedCases,
                 "Get the current number of exposed cases")
            .def("CountInfectiousCases", &stride::MDP::CountInfectiousCases,
                 "Get the current number of infectious cases")
            .def("CountSymptomaticCases", &stride::MDP::CountSymptomaticCases,
                 "Get the current number of symptomatic cases")
            .def("SimulateDay", &stride::MDP::SimulateDay,
                 "Runs the simulator for a day")
            .def("Simulate", &stride::MDP::Simulate, py::arg("numDays"),
                 "Runs the simulator for the given number of days")
            .def("SimulateVaccinate", &stride::MDP::SimulateVaccinate,
                 py::arg("numDays"), py::arg("availableVaccines"), py::arg("ageGroup"), py::arg("vaccineType"),
                 "Runs the simulator for the given number of days and vaccinates people with the given vaccine type")
            .def("Vaccinate", &stride::MDP::Vaccinate,
                 py::arg("availableVaccines"), py::arg("ageGroup"), py::arg("vaccineType"),
                 "Vaccinate a given age group with the given vaccine type for the available number of vaccines")
            .def("End", &stride::MDP::End,
                 "Signal for the simulator (and loggers) to end the experiment");
}
