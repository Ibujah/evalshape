/*
Copyright (c) 2016 Bastien Durix

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


/**
 *  \file BranchFile.cpp
 *  \brief Defines branch reader and writer
 *  \author Bastien Durix
 */

#include "BranchFile.h"
#include <mathtools/application/Bspline.h>
#include <fstream>

void fileio::WriteBranch(const skeleton::BranchContSkel3d::Ptr contbr, const std::string &filename)
{
	
	std::ofstream ofs(filename);
	
	if(ofs)
	{
		mathtools::application::Bspline<4>::Ptr bsfun =  std::static_pointer_cast<mathtools::application::Bspline<4> >(contbr->getCompFun()->getFun());

		if(bsfun)
		{
			ofs << bsfun->getDegree() << std::endl;

			ofs << bsfun->getNodeVec().cols() << std::endl;
			for(unsigned int i = 0; i < bsfun->getNodeVec().cols(); i++)
				ofs << bsfun->getNodeVec()[i] << " ";
			ofs << std::endl;

			ofs << bsfun->getCtrl().cols() << std::endl;
			for(unsigned int i = 0; i < bsfun->getCtrl().cols(); i++)
			{
				for(unsigned int j = 0; j < 4; j++)
				{
					ofs << bsfun->getCtrl()(j,i) << " ";
				}
				ofs << std::endl;
			}
		}
		ofs.close();
	}
}

skeleton::BranchContSkel3d::Ptr fileio::ReadBranch(const std::string &filename)
{
	mathtools::application::Bspline<4>::Ptr bsfun;
	std::ifstream ifs(filename);
	
	if(ifs)
	{
		unsigned int degree;
		ifs >> degree;
		
		unsigned int nbnod;
		ifs >> nbnod;
		
		Eigen::Matrix<double,1,Eigen::Dynamic>   nodevec(1,nbnod);
		for(unsigned int i = 0; i < nbnod; i++)
		{
			ifs >> nodevec(i);
		}
		
		unsigned int nbctrl;
		ifs >> nbctrl;
		
		Eigen::Matrix<double,4,Eigen::Dynamic> ctrlpt(4,nbctrl);
		for(unsigned int i = 0; i < nbctrl; i++)
		{
			for(unsigned int j = 0; j < 4; j++)
			{
				ifs >> ctrlpt(j,i);
			}
		}
		
		bsfun = mathtools::application::Bspline<4>::Ptr(new mathtools::application::Bspline<4>(ctrlpt,nodevec,degree));
		
		ifs.close();
	}
	
	return skeleton::BranchContSkel3d::Ptr(
			new skeleton::BranchContSkel3d(
				skeleton::model::Classic<3>::Ptr(new skeleton::model::Classic<3>()),
				std::static_pointer_cast<skeleton::BranchContSkel3d::NodeFun>(bsfun)));
}

