#ifndef PYMESHLAB_MESHSET_H
#define PYMESHLAB_MESHSET_H

#include <pybind11/pybind11.h>
#include <meshmodel.h>
#include <pluginmanager.h>
#include "namebinder.h"


namespace pymeshlab {

class MeshSet : public MeshDocument
{
public:
	MeshSet();

	void loadMesh(const std::string& filename, pybind11::kwargs kwargs);
	void saveMesh(const std::string& filename, pybind11::kwargs kwargs);

	void applyFilter(const std::string& filtername, pybind11::kwargs kwargs);

private:

	void updateRichParameterSet(const pybind11::kwargs& kwargs, RichParameterSet& rps);

	std::string basePath;
	RichParameterSet globalRPS;
	PluginManager pm;
	NameBinder nameBinder;
};

}

#endif // PYMESHLAB_MESHSET_H
