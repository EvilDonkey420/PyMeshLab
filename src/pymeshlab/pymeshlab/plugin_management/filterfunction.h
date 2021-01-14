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
#ifndef PYMESHLAB_FILTERFUNCTIONLIST_H
#define PYMESHLAB_FILTERFUNCTIONLIST_H

#include "filterfunctionparameter.h"

#include <QString>
#include <list>
#include <set>

namespace pymeshlab {

class FilterFunction
{
public:
	FilterFunction();
	FilterFunction(const QString pythonFunName, const QString meshlabFunName, const QString description);
	void addParameter(const FilterFunctionParameter& p);
	QString pythonFunctionName() const;
	QString meshlabFunctionName() const;
	QString description() const;
	unsigned int parametersNumber() const;
	QStringList pythonFunctionParameters() const;
	bool contains(const QString& pythonParameter) const;
	const FilterFunctionParameter& getFilterFunctionParameter(const QString& pythonParameter) const;

	bool operator< (const FilterFunction& oth) const;

	using iterator = std::list<FilterFunctionParameter>::const_iterator;

	iterator begin() const {return parameters.begin();}
	iterator end() const {return parameters.end();}

private:
	QString pythonFunName;
	QString meshlabFunName;
	QString funDescription;
	std::list<FilterFunctionParameter> parameters;
};

}

#endif // PYMESHLAB_FILTERFUNCTIONLIST_H
