#include "pymeshset.h"

#include "pymeshlabcommon.h"
#include "meshset.h"
#include <vcg/../wrap/io_trimesh/import_obj.h>
#include <pybind11/eval.h>
#include <mlexception.h>

namespace py = pybind11;

void pymeshlab::initMeshSet(pybind11::module& m)
{
	py::class_<MeshSet> meshSetClass(m, "MeshSet");

	py::register_exception<MLException>(m, "PyMeshLabException");

	//empty constructor
	meshSetClass.def(py::init());

	meshSetClass.def("number_meshes", &MeshSet::size);
	meshSetClass.def("set_current_mesh", &MeshSet::setCurrentMesh, py::arg("new_curr_id"));
	meshSetClass.def("current_mesh", &MeshSet::currentMesh, py::return_value_policy::reference);
	meshSetClass.def("current_mesh_id", &MeshSet::currentMeshId);
	meshSetClass.def("load_mesh", &MeshSet::loadMesh, py::arg("file_name"));
	meshSetClass.def("save_mesh", &MeshSet::saveMesh, py::arg("file_name"));
	meshSetClass.def("apply_filter", &MeshSet::applyFilter, py::arg("filter_name"));
	meshSetClass.def("print_filter_list", &MeshSet::printPythonFilterNamesList);
	meshSetClass.def("print_filter_parameter_list", &MeshSet::printPythonFilterParameterList, py::arg("filter_name"));
}
