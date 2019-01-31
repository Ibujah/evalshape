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
 *  \file ExportOCaml.cpp
 *  \brief Defines OCaml exporter
 *  \author Bastien Durix
 */

#include "ExportOCaml.h"
#include <fstream>
#include <mathtools/geometry/euclidian/HyperSphere.h>

void fileio::ExportOCaml(const skeleton::CompGraphSkel2d::Ptr skel, const std::string &filename)
{
	std::ofstream ofs(filename);
	
	if(ofs)
	{
		std::string name = filename.substr(0,filename.find("."));
		std::list<unsigned int> l_nod;
		std::map<unsigned int,unsigned int> m_nod;
		skel->getAllNodes(l_nod);
		
		unsigned int nb_nod = 1;
		
		ofs << "let " << name << " = create ();;" << std::endl;
		ofs << std::endl;

		for(std::list<unsigned int>::iterator it = l_nod.begin(); it != l_nod.end(); it++)
		{
			std::list<unsigned int> l_br;
			skel->getNeighbors(*it,l_br);
			mathtools::geometry::euclidian::HyperSphere<2> sph = skel->getBranch(*it,*(l_br.begin()))->getNode<mathtools::geometry::euclidian::HyperSphere<2> >(0);
			ofs << "let " << name << "v" << nb_nod << " = "
						   << "createv (" << nb_nod << ", (" << sph.getCenter().getCoords().x() << ", " << sph.getCenter().getCoords().y() << "), " << sph.getRadius() << ");;" << std::endl;
			m_nod[*it] = nb_nod;
			nb_nod++;
		}
		ofs << std::endl;
		
		for(unsigned int i = 1; i < nb_nod; i++)
		{
			ofs << "add_vertex " << name << " " << name << "v" << i << ";;" << std::endl;
		}
		ofs << std::endl;
		
		std::list<unsigned int> l_edg;
		skel->getAllEdges(l_edg);

		for(std::list<unsigned int>::iterator it = l_edg.begin(); it != l_edg.end(); it++)
		{
			std::pair<unsigned int,unsigned int> ext = skel->getExtremities(*it);
			ofs << "add_edge " << name << " " << name << "v" << m_nod[ext.first] << " " << name << "v" << m_nod[ext.second] << ";;" << std::endl;
		}
		ofs << std::endl;
		
		ofs.close();
	}
	else
	{
		std::cout << "Opening file error " << filename << std::endl;
	}
}
