#include "meshset.h"

#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <common/mlexception.h>
#include <common/meshlabdocumentxml.h>
#include <common/meshlabdocumentbundler.h>
#include <wrap/io_trimesh/alnParser.h>

#include "percentage.h"
#include "exceptions.h"
#include "helpers/common.h"
#include "helpers/meshlab_singletons.h"
#include "helpers/meshset_helper.h"
#include "helpers/verbosity_manager.h"

namespace py = pybind11;

pymeshlab::MeshSet::MeshSet(bool verbose) :
	MeshDocument(),
	pm(MeshLabSingletons::pluginManagerInstance(verbose))
{
	filterFunctionSet.populate(pm);
	setVerbosity(verbose);
}

pymeshlab::MeshSet::~MeshSet()
{
}

void pymeshlab::MeshSet::setVerbosity(bool verbose)
{
	this->verbose = verbose;
	if (verbose){
		for (auto p : pm.ownerPlug)
			p->setLog(&Log);
	}
	else {
		for (auto p : pm.ownerPlug)
			p->setLog(nullptr);
	}
}

CMeshO& pymeshlab::MeshSet::currentMesh()
{
	if (mm() == nullptr)
		throw MLException("MeshSet has no selected Mesh.");
	return mm()->cm;
}

int pymeshlab::MeshSet::currentMeshId() const
{
	if (mm() == nullptr)
		throw MLException("MeshSet has no selected Mesh.");
	return mm()->id();
}

bool pymeshlab::MeshSet::meshIdExists(int id) const
{
	return getMesh(id) != nullptr;
}

CMeshO& pymeshlab::MeshSet::mesh(int id)
{
	MeshModel* tmp = getMesh(id);
	if (tmp == nullptr)
		throw MLException("Mesh ID " + QString::number(id) + " not found in MeshSet.");
	return tmp->cm;
}

void pymeshlab::MeshSet::printPluginList() const
{
	std::cout 
			<< "MeshSet class - list of loaded plugins (" 
			<< pm.ownerPlug.size() << "):\n";
	for (const PluginInterface* p : pm.ownerPlug){
		std::cout << "\t" << p->pluginName().toStdString() << "\n";
	}
}

/**
 * @brief lists all the filters that can be called using the "apply_filter" function
 */
void pymeshlab::MeshSet::printPythonFilterNamesList() const
{
	QStringList list = filterFunctionSet.pythonFunctionNames();
	std::cout << "MeshSet class - list of filter names:\n";
	for (const QString& fname : list){
		if (!fname.startsWith("load_") && !fname.startsWith("save_"))
			std::cout << "\t" << fname.toStdString() << "\n";
	}
}

/**
 * @brief given the function names, lists all its parameters that can be 
 * passed to the "apply_filter" function
 * @param functionName
 */
void pymeshlab::MeshSet::printPythonFilterParameterList(const std::string functionName) const
{
	if (meshsethelper::pythonFilterNameExists(functionName, filterFunctionSet)){
		std::cout << "Filter " << functionName << " not found.\n";
	}
	else {
		FilterFunctionSet::iterator it = filterFunctionSet.find(QString::fromStdString(functionName));
		std::cout <<
					"Please note: some parameters depend on the mesh(es) used as input of the \n"
					"filter. Default values listed here are computed on a 1x1x1 cube \n"
					"(pymeshlab/tests/sample/cube.obj), and they will be computed on the input mesh\n"
					"if they are left as default.\n";

		std::cout << functionName <<" filter - list of parameter names:\n";
		const FilterFunction& ff = *it;
		if (ff.parametersNumber() == 0) {
			std::cout << "\tNone.\n";
		}
		else {
			for (const FilterFunctionParameter& ffp : ff) {
				std::cout << "\t" << ffp.pythonName().toStdString() << " : "
						  << ffp.pythonTypeString().toStdString() << " = ";
				ffp.printDefaultValue(std::cout);
				std::cout << "\n";
			}
		}
	}
}

/**
 * @brief prints all the filters (with their parameters) that have been applied on the current 
 * MeshSet. 
 */
void pymeshlab::MeshSet::printFilterScript() const
{
	std::cout << "Filter Script Size : " << filterScript.size() << "\n\n";
	uint i = 0;
	for (const FilterNameParameterValuesPair& p :filterScript){
		std::cout << std::to_string(i) + ": " << FilterFunctionSet::toPythonName(p.filterName()).toStdString() <<
					 "\n";
		for (const RichParameter& par : p.second){
			FilterFunctionParameter ffp(FilterFunctionSet::toPythonName(par.name()), par);
			std::cout << "\t" << ffp.pythonName().toStdString() << " : "
					  << ffp.pythonTypeString().toStdString() << " = ";
			ffp.printDefaultValue(std::cout);
			std::cout << "\n";
		}
		std::cout << "\n";
		i++;
	}
}

void pymeshlab::MeshSet::loadNewMesh(const std::string& filename, py::kwargs kwargs)
{
	meshsethelper::loadMeshUsingPlugin(filename, nullptr, kwargs, *this, filterFunctionSet);
}

void pymeshlab::MeshSet::loadCurrentMesh(const std::string& filename, pybind11::kwargs kwargs)
{
	if (mm() == nullptr)
		throw MLException("MeshSet has no selected Mesh.");
	mm()->Clear();
	meshsethelper::loadMeshUsingPlugin(filename, mm(), kwargs, *this, filterFunctionSet);
}

void pymeshlab::MeshSet::saveCurrentMesh(const std::string& filename, pybind11::kwargs kwargs)
{
	if (mm() == nullptr)
		throw MLException("MeshSet has no selected Mesh.");
	meshsethelper::saveMeshUsingPlugin(filename, mm(), kwargs, *this, filterFunctionSet);
}

/**
 * @brief adds the reference of the given mesh to the MeshSet.
 * @param mesh
 * @param name
 * @param setAsCurrent: default value: true
 */
void pymeshlab::MeshSet::addMesh(CMeshO& mesh, const std::string& name, bool setAsCurrent)
{
	MeshModel* mm = this->addNewMesh("", QString::fromStdString(name), setAsCurrent);
	mm->cm = mesh;
}

void pymeshlab::MeshSet::loadProject(const std::string& filename)
{
	QString fileName = QString::fromStdString(filename);
	if (filename.empty())
		throw MLException("file_name empty.");

	QFileInfo fi(fileName);
	if((fi.suffix().toLower()!="aln") &&
			(fi.suffix().toLower()!="mlp")  &&
			(fi.suffix().toLower() != "mlb") &&
			(fi.suffix().toLower()!="out") &&
			(fi.suffix().toLower()!="nvm")) {
		throw MLException("Unknown project file extension: " + fi.suffix().toLower());
	}

	setFileName(fileName);
	setDocLabel(fileName);

	if (QString(fi.suffix()).toLower() == "aln") {
		loadALN(fileName);
	}

	if (QString(fi.suffix()).toLower() == "mlp" || QString(fi.suffix()).toLower() == "mlb") {
		loadMLP(fileName);
	}

	if (QString(fi.suffix()).toLower() == "out"){
		loadBundler(fileName);
	}

	if (QString(fi.suffix()).toLower() == "nvm"){
		loadNVM(fileName);
	}
}

void pymeshlab::MeshSet::saveProject(const std::string& filename)
{
	QString fileName = QString::fromStdString(filename);
	if (filename.empty())
		throw MLException("file_name empty.");

	QFileInfo fi(fileName);
	if((fi.suffix().toLower()!="mlp")  &&
			(fi.suffix().toLower() != "mlb")) {
		throw MLException("Unknown project file extension: " + fi.suffix().toLower());
	}

	if (QString(fi.suffix()).toLower() == "mlp" || QString(fi.suffix()).toLower() == "mlb") {
		saveMLP(fileName);
	}
}

void pymeshlab::MeshSet::loadFilterScript(const std::string& filename)
{
	pymeshlab::QDebugRedirect* qdbr = nullptr;
	if (!verbose)
		qdbr = new pymeshlab::QDebugRedirect(); //redirect qdebug to null

	filterScript.open(QString::fromStdString(filename));

	delete qdbr;
}

void pymeshlab::MeshSet::saveFilterScript(const std::string& filename) const
{
	filterScript.save(QString::fromStdString(filename));
}

void pymeshlab::MeshSet::clearFilterScript()
{
	filterScript.clear();
}

void pymeshlab::MeshSet::applyFilterScript()
{
	for (FilterNameParameterValuesPair p : filterScript){
		QString meshlabFilterName = p.first;
		std::string filtername = FilterFunctionSet::toPythonName(meshlabFilterName).toStdString();
		QAction* action = nullptr;
		FilterPluginInterface* fp = meshsethelper::pluginFromFilterName(meshlabFilterName, action);
		RichParameterList rpl;
		fp->initParameterList(action, *this, rpl);
		meshsethelper::updateRichParameterList(filtername, p.second, rpl);
		meshsethelper::applyFilterRPL(
				filtername, meshlabFilterName, action, fp, rpl,
				verbose, filterScript, false, *this);
	}
}

pybind11::dict pymeshlab::MeshSet::applyFilter(const std::string& filtername, pybind11::kwargs kwargs)
{
	py::dict outputValues;
	FilterFunctionSet::iterator it = filterFunctionSet.find(QString::fromStdString(filtername));
	if (it != filterFunctionSet.end() && 
			!(QString::fromStdString(filtername).startsWith("load_") && 
			!(QString::fromStdString(filtername).startsWith("save_")))) {
		QString meshlabFilterName = it->meshlabFunctionName();
		
		QAction* action = nullptr;
		FilterPluginInterface* fp = meshsethelper::pluginFromFilterName(meshlabFilterName, action);
		RichParameterList rpl;
		fp->initParameterList(action, *this, rpl);
		meshsethelper::updateRichParameterListFromKwargs(*it, kwargs, this, rpl);
		outputValues = meshsethelper::applyFilterRPL(
				filtername, meshlabFilterName, action, fp, rpl,
				verbose, filterScript, true, *this);
	}
	else {
		throw MLException(
					"Failed to apply filter: " + QString::fromStdString(filtername) + "\n" +
					"Filter does not exists. Take a look at MeshSet.print_filter_list function.");
	}
	return outputValues;
}

void pymeshlab::MeshSet::printStatus() const
{
	std::cout << "Number meshes: " << size() << "\n";
	for (const MeshModel* m : meshList){
		std::cout << "\tMesh id: " << m->id() << "\n";
		std::cout << "\tMesh label: " << m->label().toStdString() << "\n";
		std::cout << "\tFull name: " << m->fullName().toStdString() << "\n\n";
	}
}

std::string pymeshlab::MeshSet::filtersRSTDocumentation() const
{
	return meshsethelper::RSTDocumentationFromFilterFunctionSet(filterFunctionSet);
}

void pymeshlab::MeshSet::loadALN(const QString& fileName)
{
	QFileInfo fi(fileName);
	QString absfilename = fi.absolutePath() + "/" + fi.fileName();
	QDir currentDir = QDir::current();
	// this change of dir is needed for subsequent textures/materials loading
	QDir::setCurrent(fi.absoluteDir().absolutePath());

	std::vector<RangeMap> rmv;
	int retVal = ALNParser::ParseALN(rmv, qUtf8Printable(absfilename));
	if(retVal != ALNParser::NoError) {
		throw MLException("Error: Unable to open ALN file: " + absfilename);
	}

	bool openRes=true;
	std::vector<RangeMap>::iterator ir;
	for(ir=rmv.begin();ir!=rmv.end() && openRes;++ir) {
		QString relativeToProj = fi.absoluteDir().absolutePath() + "/" + (*ir).filename.c_str();
		loadNewMesh(relativeToProj.toStdString(), py::kwargs());
		mm()->cm.Tr = ir->transformation;
	}

	//restore current dir
	QDir::setCurrent(currentDir.absolutePath());
}

void pymeshlab::MeshSet::loadMLP(const QString& fileName)
{
	QFileInfo fi(fileName);
	QString absfilename = fi.absolutePath() + "/" + fi.fileName();
	QDir currentDir = QDir::current();
	// this change of dir is needed for subsequent textures/materials loading
	QDir::setCurrent(fi.absolutePath());

	std::map<int, MLRenderingData> rendOpt;
	int startingIndex = meshList.size();
	if (!MeshDocumentFromXML(*this, absfilename, (QString(fi.suffix()).toLower() == "mlb"), rendOpt)) {
		throw MLException("Error:  Unable to open MeshLab Project file: " + absfilename);
	}
	for (int i=startingIndex; i<meshList.size(); i++) {
		QString fullPath = meshList[i]->fullName();
		Matrix44m trm = meshList[i]->cm.Tr;
		meshsethelper::loadMeshUsingPlugin(fullPath.toStdString(), meshList[i], pybind11::kwargs(), *this, filterFunctionSet);
		mm()->cm.Tr = trm;
	}

	//restore current dir
	QDir::setCurrent(currentDir.absolutePath());
}

void pymeshlab::MeshSet::loadBundler(const QString& fileName)
{
	QFileInfo fi(fileName);
	QString cameras_filename = fi.absolutePath() + "/" + fi.fileName();
	QString image_list_filename = fi.absolutePath() + "/list.txt";
	QString model_filename;

	QDir currentDir = QDir::current();
	// this change of dir is needed for subsequent textures/materials loading
	QDir::setCurrent(fi.absolutePath());

	if(!MeshDocumentFromBundler(*this, cameras_filename, image_list_filename, model_filename)){
		throw MLException("Error:  Unable to open OUTs file: " + cameras_filename);
	}

	//restore current dir
	QDir::setCurrent(currentDir.absolutePath());
}

void pymeshlab::MeshSet::loadNVM(const QString& fileName)
{
	QFileInfo fi(fileName);
	QString cameras_filename = fi.absolutePath() + "/" + fi.fileName();
	QString image_list_filename = fi.absolutePath() + "/list.txt";
	QString model_filename;

	QDir currentDir = QDir::current();
	// this change of dir is needed for subsequent textures/materials loading
	QDir::setCurrent(fi.absolutePath());

	if(!MeshDocumentFromNvm(*this, cameras_filename, model_filename)){
		throw MLException("Error:  Unable to open NVMs file: " + cameras_filename);
	}

	//restore current dir
	QDir::setCurrent(currentDir.absolutePath());
}

void pymeshlab::MeshSet::saveMLP(const QString& fileName)
{
	QFileInfo fi(fileName);
	QString outdir = fi.absolutePath();
	QString absfilename = outdir + "/" + fi.fileName();
	QDir currentDir = QDir::current();
	// this change of dir is needed for subsequent textures/materials loading
	QDir::setCurrent(fi.absolutePath());

	for(MeshModel* m : meshList) {
		if (m != NULL) {
			QString label = m->label().remove(" ");
			int p = label.lastIndexOf('.');
			if (p > 0)
				label = label.left(p);
			QString outfilename = outdir + "/" + label.remove(" ") + ".ply";
			m->setFileName(outfilename);
			QFileInfo of(outfilename);
			m->setLabel(of.fileName());
			meshsethelper::saveMeshUsingPlugin(outfilename.toStdString(), m, py::kwargs(), *this, filterFunctionSet);
		}
	}

	bool ok = MeshDocumentToXMLFile(*this, fileName, false, false, fi.suffix().toLower() == "mlb");
	if (!ok){
		throw MLException("Error:  Unable to save MeshLab Project file: " + absfilename);
	}
	//restore current dir
	QDir::setCurrent(currentDir.absolutePath());
}



