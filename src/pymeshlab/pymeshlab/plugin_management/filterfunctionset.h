#ifndef PYMESHLAB_NAMEBINDER_H
#define PYMESHLAB_NAMEBINDER_H

#include <common/pluginmanager.h>
#include "filterfunction.h"

namespace pymeshlab {

class FilterFunctionSet
{
public:
	FilterFunctionSet();
	void populate(const PluginManager& pm);

	QStringList pythonFunctionNames() const;

	using iterator = std::set<FilterFunction>::iterator;

	iterator begin() const {return functionSet.begin();}
	iterator end() const {return functionSet.end();}

	iterator find(const QString& pythonFunctionName) const;
	bool contains(const QString& pythonFunctionName) const;

	static QString toPythonName(const QString& name);

private:
	void addFunction(const FilterFunction& f);
	void deleteFunction(const FilterFunction& f);
	void updateSaveParameters(
			IOMeshPluginInterface* plugin,
			const QString& outputFormat,
			FilterFunction& f);
	std::set<FilterFunction> functionSet;
};

}

#endif // PYMESHLAB_NAMEBINDER_H
