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
 *  \file ExportSVG.cpp
 *  \brief Defines SVG exporter
 *  \author Bastien Durix
 */

#include "ExportSVG.h"
#include <fstream>
#include <mathtools/geometry/euclidian/HyperSphere.h>

#define FAC 4

void fileio::ExportSVG(const skeleton::CompGraphSkel2d::Ptr skel, const std::string &filename, unsigned int width, unsigned int height)
{
	std::ofstream ofs(filename);
	
	if(ofs)
	{
		std::list<unsigned int> l_nod;
		skel->getAllNodes(l_nod);
		std::list<unsigned int> l_edg;
		skel->getAllEdges(l_edg);
		std::string name = filename.substr(0,filename.find("."));
		
		ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << std::endl;
		ofs << "<svg height=\"" << FAC*height << "px\" width=\"" << FAC*width << "px\">" << std::endl;
		ofs << "<g>" << std::endl;
		
		ofs << "<image xlink:href=\"./" << name << ".png\" x=\"0\" y=\"0\" height=\"" << FAC*height << "\" width=\"" << FAC*width << "\"/>" << std::endl;

		for(std::list<unsigned int>::iterator it = l_edg.begin(); it != l_edg.end(); it++)
		{
			mathtools::geometry::euclidian::HyperSphere<2> sph1 = skel->getBranch(*it)->getNode<mathtools::geometry::euclidian::HyperSphere<2> >(0);
			mathtools::geometry::euclidian::HyperSphere<2> sph2 = skel->getBranch(*it)->reverted()->getNode<mathtools::geometry::euclidian::HyperSphere<2> >(0);
			ofs << "<path d=\"M " << FAC*sph1.getCenter().getCoords().x() << " " << FAC*sph1.getCenter().getCoords().y() << " "
							 "L " << FAC*sph2.getCenter().getCoords().x() << " " << FAC*sph2.getCenter().getCoords().y() << " "
								  << "\" style=\"stroke:#000000;stroke-width:4\" />" << std::endl;
		}
		ofs << std::endl;

		for(std::list<unsigned int>::iterator it = l_nod.begin(); it != l_nod.end(); it++)
		{
			std::list<unsigned int> l_br;
			skel->getNeighbors(*it,l_br);
			mathtools::geometry::euclidian::HyperSphere<2> sph = skel->getBranch(*it,*(l_br.begin()))->getNode<mathtools::geometry::euclidian::HyperSphere<2> >(0);
			ofs << "<circle"
					  //<< "  r=\"" << sph.getRadius() << "\""
					  << "  r=\"20px\""
					  << " cy=\"" << FAC*sph.getCenter().getCoords().y() << "\""
					  << " cx=\"" << FAC*sph.getCenter().getCoords().x() << "\""
					  << " style=\"color:#000000;fill:#ffffff;stroke:#000000;stroke-width:4\" />" << std::endl;
			ofs << "<text y=\"" << FAC*(sph.getCenter().getCoords().y()) << "\""
					 << " x=\"" << FAC*(sph.getCenter().getCoords().x()-3) << "\""
					 << " style=\"font-size:15px\">" << (*it)+1 << "</text>" << std::endl;
		}
		ofs << std::endl;
		
		ofs << std::endl;
		ofs << "</g>" << std::endl;
		ofs << "</svg>" << std::endl;
		
		ofs.close();
	}
	else
	{
		std::cout << "Opening file error " << filename << std::endl;
	}
}
