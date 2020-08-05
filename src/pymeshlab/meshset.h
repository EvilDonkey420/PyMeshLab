#ifndef PYMESHLAB_MESHSET_H
#define PYMESHLAB_MESHSET_H

#include <pybind11/pybind11.h>
#include <meshmodel.h>
#include <pluginmanager.h>
#include "filterfunctionset.h"


namespace pymeshlab {

class MeshSet : public MeshDocument
{
public:
	MeshSet();
	virtual ~MeshSet();

	CMeshO& currentMesh();
	int currentMeshId() const;

	void printPythonFilterNamesList() const;
	void printPythonFilterParameterList(const std::string functionName) const;

	void loadMesh(const std::string& filename, pybind11::kwargs kwargs);
	void saveMesh(const std::string& filename, pybind11::kwargs kwargs);

	void applyFilter(const std::string& filtername, pybind11::kwargs kwargs);

private:
	void updateRichParameterSet(
			const FilterFunction& f,
			const pybind11::kwargs& kwargs,
			RichParameterList& rps,
			bool ignoreFileName = false);

	static void updateRichParameterFromKwarg(
			RichParameter& par,
			const FilterFunctionParameter& ffp,
			const std::pair<pybind11::handle, pybind11::handle>& k);

	MeshFilterInterface* getPluginFromFilterName(
			const QString& filterName,
			QAction*& action) const;

	std::string basePath;
	RichParameterList globalRPS;
	PluginManager pm;
	FilterFunctionSet filterFunctionSet;
};

}

#endif // PYMESHLAB_MESHSET_H
