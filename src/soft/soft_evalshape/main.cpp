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
 *  \brief 2D shape evaluation
 *  \author Bastien Durix
 */

#include <boost/program_options.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

#include <shape/DiscreteShape.h>
#include <boundary/DiscreteBoundary.h>

#include <algorithm/extractboundary/NaiveBoundary.h>
#include <algorithm/evaluation/ShapeError.h>

int main(int argc, char** argv)
{
	std::string imgref, imgcmp;

	boost::program_options::options_description desc("OPTIONS");
	
	desc.add_options()
		("help", "Help message")
		("imgref", boost::program_options::value<std::string>(&imgref)->default_value("img1.png"), "Reference binary image file")
		("imgcmp", boost::program_options::value<std::string>(&imgcmp)->default_value("img2.png"), "Compaired binary image file")
		;
	
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	
	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	cv::Mat shprefgray = cv::imread(imgref,cv::ImreadModes::IMREAD_GRAYSCALE);
	cv::Mat shpref;
	cv::threshold(shprefgray,shpref,1,255,cv::THRESH_BINARY);
	shape::DiscreteShape<2>::Ptr disshref = shape::DiscreteShape<2>::Ptr(new shape::DiscreteShape<2>(shpref.cols,shpref.rows));

	cv::Mat shpcmpgray = cv::imread(imgcmp,cv::ImreadModes::IMREAD_GRAYSCALE);
	cv::Mat shpcmp;
	cv::threshold(shpcmpgray,shpcmp,1,255,cv::THRESH_BINARY);
	shape::DiscreteShape<2>::Ptr disshcmp = shape::DiscreteShape<2>::Ptr(new shape::DiscreteShape<2>(shpcmp.cols,shpcmp.rows));
	
	double symdiff = algorithm::evaluation::SymDiffArea(disshref,disshcmp);

	std::cout << "Symmetric area difference :  " << symdiff << std::endl;

	boundary::DiscreteBoundary<2>::Ptr bndref = algorithm::extractboundary::NaiveBoundary(disshref);
	boundary::DiscreteBoundary<2>::Ptr bndcmp = algorithm::extractboundary::NaiveBoundary(disshcmp);

	double hausdist = algorithm::evaluation::HausDist(bndref,bndcmp);

	std::cout << "Hausdorff Distance :  " << hausdist << std::endl;

	return 0;
}
