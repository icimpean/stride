#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_mdp(py::module &);

// Export the library pylibstride as a module
PYBIND11_MODULE(pylibstride, m) {
    m.doc() = "STRIDE module";

    init_mdp(m);
}
