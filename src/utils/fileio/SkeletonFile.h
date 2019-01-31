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
 *  \file SkeletonFile.h
 *  \brief Defines skeleton file writer
 *  \author Bastien Durix
 */

#include <skeleton/Skeletons.h>

/**
 *  \brief Gives file input and output functions
 */
namespace fileio
{
	/**
	 *  \brief Writes a skeleton file, in .obj format
	 *
	 *  \param skel     3D skeleton to write
	 *  \param filename  skeleton filename
	 */
	void WriteSkeletonOBJ(const skeleton::GraphSkel3d::Ptr skel, const std::string &filename);

	void WriteSkeleton2D(const skeleton::GraphSkel2d::Ptr skel, const std::map<unsigned int,std::list<unsigned int> >& pt_assoc_bnd, const std::string &filename);

	void WriteSkeleton2Dpts(const skeleton::GraphSkel2d::Ptr skel, const std::map<unsigned int,std::list<unsigned int> >& pt_assoc_bnd, const std::string &filename);
	void WriteSkeleton2Dedg(const skeleton::GraphSkel2d::Ptr skel, const std::map<unsigned int,std::list<unsigned int> >& pt_assoc_bnd, const std::string &filename);
	void WriteSkeleton2Dbnd(const skeleton::GraphSkel2d::Ptr skel, const std::map<unsigned int,std::list<unsigned int> >& pt_assoc_bnd, const std::string &filename);
}


