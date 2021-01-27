/****************************************************************************
* PyMeshLab                                                         o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005-2021                                           \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
#include "filterfunctionset.h"

#include <QRegularExpression>
#include <algorithm>
#include "pymeshlab/helpers/common.h"

pymeshlab::FilterFunctionSet::FilterFunctionSet()
{
}

void pymeshlab::FilterFunctionSet::populate(const PluginManager& pm)
{
	std::string samplesPath = getSamplesPath();

	MeshDocument dummyMeshDocument;
	QString dummyMesh(QString::fromStdString(getSamplesPath() + "cube.obj"));
	dummyMeshDocument.addNewMesh(dummyMesh, "cube");
	int mask = 0;
	mask |= vcg::tri::io::Mask::IOM_VERTQUALITY;
	mask |= vcg::tri::io::Mask::IOM_FACEQUALITY;
	dummyMeshDocument.mm()->Enable(mask);

	for (auto inputFormat : pm.inputMeshFormatList()){
		QString originalFilterName = inputFormat;
		QString pythonFilterName = "load_" + inputFormat.toLower();
		FilterFunction f(pythonFilterName, originalFilterName, "Load " + inputFormat + " format.");
		IOMeshPluginInterface* plugin = pm.inputMeshPlugin(inputFormat);
		RichParameterList rps;
		plugin->initPreOpenParameter(inputFormat, dummyMesh, rps);
		plugin->initOpenParameter(inputFormat, *dummyMeshDocument.mm(), rps);

		//filename parameter
		QString pythonParameterName = "file_name";
		QString sv = "file_name." + inputFormat;
		QStringList sl(inputFormat);
		RichOpenFile of("fileName", sv, sl, "File Name", "The name of the file to load");
		FilterFunctionParameter par(pythonParameterName, of);
		f.addParameter(par);

		for (const RichParameter& rp : rps){
			QString pythonParameterName = toPythonName(rp.name());
			FilterFunctionParameter par(pythonParameterName, rp);
			f.addParameter(par);
		}
		functionSet.insert(f);
	}

	for (auto outputFormat : pm.outputMeshFormatList()){
		QString originalFilterName = outputFormat;
		QString pythonFilterName = "save_" + outputFormat.toLower();
		FilterFunction f(pythonFilterName, originalFilterName, "Save " + outputFormat + " format.");
		IOMeshPluginInterface* plugin = pm.outputMeshPlugin(outputFormat);
		RichParameterList rps;
		plugin->initSaveParameter(outputFormat, *dummyMeshDocument.mm(), rps);

		//filename parameter
		QString pythonParameterName = "file_name";
		QString sv = "file_name." + outputFormat;
		RichSaveFile of("fileName", sv, outputFormat, "File Name", "The name of the file to save");
		FilterFunctionParameter par(pythonParameterName, of);
		f.addParameter(par);

		for (const RichParameter& rp : rps){
			QString pythonParameterName = toPythonName(rp.name());
			FilterFunctionParameter par(pythonParameterName, rp);
			f.addParameter(par);
		}

		//data to save
		updateSaveParameters(plugin, outputFormat, f);

		functionSet.insert(f);
	}
	
	for (auto inputRasterFormat : pm.inputRasterFormatList()){
		QString originalFilterName = inputRasterFormat;
		QString pythonFilterName = "loadr_" + inputRasterFormat.toLower();
		FilterFunction f(pythonFilterName, originalFilterName, "Load " + inputRasterFormat + " format.");

		//filename parameter
		QString pythonParameterName = "file_name";
		QString sv = "file_name." + inputRasterFormat;
		QStringList sl(inputRasterFormat);
		RichOpenFile of("fileName", sv, sl, "File Name", "The name of the file to load");
		FilterFunctionParameter par(pythonParameterName, of);
		f.addParameter(par);

		functionSet.insert(f);
	}

	for (FilterPluginInterface* fp : pm.filterPluginIterator()){
		QList<QAction*> acts = fp->actions();
		for (QAction* act : acts) {
			QString originalFilterName = fp->filterName(act);
			QString description = fp->filterInfo(act);
			QString pythonFilterName = toPythonName(originalFilterName);
			FilterFunction f(pythonFilterName, originalFilterName, description);

			RichParameterList rps;
			fp->initParameterList(act, dummyMeshDocument, rps);

			for (const RichParameter& rp : rps){
				QString pythonParameterName = toPythonName(rp.name());
				FilterFunctionParameter par(pythonParameterName, rp);
				f.addParameter(par);
			}
			functionSet.insert(f);
		}
	}
}

QStringList pymeshlab::FilterFunctionSet::pythonFunctionNames() const
{
	QStringList fnames;
	for (const FilterFunction& f: functionSet){
		fnames.push_back(f.pythonFunctionName());
	}
	return fnames;
}

pymeshlab::FilterFunctionSet::iterator pymeshlab::FilterFunctionSet::find(const QString& pythonFunctionName) const
{
	return functionSet.find(FilterFunction(pythonFunctionName, "", ""));
}

bool pymeshlab::FilterFunctionSet::contains(const QString& pythonFunctionName) const
{
	return find(pythonFunctionName) != end();
}

QString pymeshlab::FilterFunctionSet::toPythonName(const QString& name)
{
	QString pythonName = name.toLower();
	pythonName.replace(' ', '_');
	pythonName.replace('/', '_');
	pythonName.replace('-', '_');
	pythonName.remove(QRegularExpression("[().,'\":+]+"));
	
	if (pythonKeywords.contains(pythonName))
		pythonName += "_";
	return pythonName;
}

void pymeshlab::FilterFunctionSet::addFunction(const pymeshlab::FilterFunction& f)
{
	functionSet.insert(f);
}

void pymeshlab::FilterFunctionSet::deleteFunction(const pymeshlab::FilterFunction& f)
{
	functionSet.erase(f);
}

void pymeshlab::FilterFunctionSet::updateSaveParameters(IOMeshPluginInterface* plugin,
		const QString& outputFormat,
		pymeshlab::FilterFunction& f)
{
	int capabilityBits, defaultBits;
	plugin->GetExportMaskCapability(outputFormat, capabilityBits, defaultBits);

	for (unsigned int i = 0; i < capabilitiesBits.size(); ++i){
		if (capabilityBits & capabilitiesBits[i]){
			bool def = defaultBits & capabilitiesBits[i];
			RichBool rb(
						saveCapabilitiesStrings[i], def,
						saveCapabilitiesStrings[i], saveCapabilitiesStrings[i]);
			FilterFunctionParameter par(toPythonName(saveCapabilitiesStrings[i]), rb);
			f.addParameter(par);

		}
	}


}
