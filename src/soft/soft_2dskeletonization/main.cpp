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
 *  \brief 2D skeletonization
 *  \author Bastien Durix
 */

#include <boost/program_options.hpp>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <chrono>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

#include <shape/DiscreteShape.h>
#include <boundary/DiscreteBoundary.h>
#include <skeleton/Skeletons.h>

#include <algorithm/extractboundary/NaiveBoundary.h>
#include <algorithm/skeletonization/propagation/SpherePropagation2D.h>
#include <algorithm/skinning/Filling.h>
#include <algorithm/evaluation/ReprojError.h>
#include <algorithm/pruning/ScaleAxisTransform.h>
#include <algorithm/pruning/LambdaMedialAxis.h>
#include <algorithm/pruning/ThetaMedialAxis.h>

#include <displayopencv/DisplayShapeOCV.h>
#include <displayopencv/DisplayBoundaryOCV.h>
#include <displayopencv/DisplaySkeletonOCV.h>

#include <fileio/SkeletonFile.h>
#include <fileio/BoundaryFile2D.h>

std::tuple<double,double,int,int> EvalSkel(const shape::DiscreteShape<2>::Ptr dissh,
									   const boundary::DiscreteBoundary<2>::Ptr disbnd,
									   const skeleton::GraphSkel2d::Ptr skel)
{
	shape::DiscreteShape<2>::Ptr shp(new shape::DiscreteShape<2>(dissh->getWidth(),dissh->getHeight()));
	algorithm::skinning::Filling(shp,skel);
	
	double res = algorithm::evaluation::SymDiffArea(dissh,shp);
	double res2 = algorithm::evaluation::HausDist(skel,disbnd,dissh->getFrame());
	
	std::list<unsigned int> lnod;
	skel->getAllNodes(lnod);
	unsigned int nbbr = 0;
	for(std::list<unsigned int>::iterator it = lnod.begin(); it != lnod.end(); it++)
	{
		unsigned int deg = skel->getNodeDegree(*it);
		if(deg != 2)
			nbbr += deg;
	}
	nbbr /= 2;
	std::tuple<double,double,int,int> result = std::make_tuple(res*100.0,res2,skel->getNbNodes(),nbbr);
	
	return result;
}

int main(int argc, char** argv)
{
	std::string imgfile, fileskl, fileimg, filebnd;
	bool output = false;
	bool eval = false;
	double alpha;

	boost::program_options::options_description desc("OPTIONS");
	
	desc.add_options()
		("help", "Help message")
		("imgfile", boost::program_options::value<std::string>(&imgfile)->default_value("mask"), "Binary image file (*.png)")
		("output", boost::program_options::value<bool>(&output)->implicit_value(true), "Returns output images")
		("alpha", boost::program_options::value<double>(&alpha)->default_value(2.1), "Skeleton precision")
		("fileskl", boost::program_options::value<std::string>(&fileskl)->default_value(""), "Skeleton file")
		("filebnd", boost::program_options::value<std::string>(&filebnd)->default_value(""), "Boundary file")
		("fileimg", boost::program_options::value<std::string>(&fileimg)->default_value("skelpropagortho"), "Skeleton img file")
		("eval", boost::program_options::value<bool>(&eval)->implicit_value(true), "Do evaluation")
		;
	
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	
	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	std::ostringstream imgfilename;
	imgfilename << imgfile << ".png";

	time_t start,end;
	double diff;

	cv::Mat shpimggray = cv::imread(imgfilename.str(),cv::ImreadModes::IMREAD_GRAYSCALE);
	cv::Mat shpimg;
	cv::threshold(shpimggray,shpimg,1,255,cv::THRESH_BINARY);
	/*cv::Mat labels;
	cv::connectedComponents(shpimg,labels,4,CV_16U);
	for(int l = 0; l < shpimg.rows; l++)
		for(int c = 0; c < shpimg.cols; c++)
		{
			if(labels.at<uint16_t>(l,c) == labels.at<uint16_t>(0,0) ||
			   labels.at<uint16_t>(l,c) == labels.at<uint16_t>(labels.rows-1,0) ||
			   labels.at<uint16_t>(l,c) == labels.at<uint16_t>(0,labels.cols-1) ||
			   labels.at<uint16_t>(l,c) == labels.at<uint16_t>(labels.rows-1,labels.cols-1))
				shpimg.at<char>(l,c) = 0;
			else
				shpimg.at<char>(l,c) = 255;
		}*/
	

	cv::Mat shpdil;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(3,3),cv::Point(1,1));
	cv::dilate(shpimg,shpdil,element);
	cv::erode(shpdil,shpimg,element);
	shape::DiscreteShape<2>::Ptr dissh = shape::DiscreteShape<2>::Ptr(new shape::DiscreteShape<2>(shpimg.cols,shpimg.rows));
	cv::Mat cpymat(shpimg.rows,shpimg.cols,CV_8U,&dissh->getContainer()[0]);
	shpimg.copyTo(cpymat);
	cv::Mat image(shpimg.rows,shpimg.cols,CV_8UC3,cv::Scalar(0,0,0));
	
	displayopencv::DisplayDiscreteShape(dissh,image,dissh->getFrame(),cv::Scalar(255,255,255));
	
	//if(output)
	//{
	//	std::ostringstream oss;
	//	oss.setf( std::ios::fixed, std:: ios::floatfield );
	//	oss.precision(1);
	//	oss << fileimg << ".png";
	//	cv::imwrite(oss.str(), image);
	//}
	
	boundary::DiscreteBoundary<2>::Ptr disbnd = algorithm::extractboundary::NaiveBoundary(dissh);

	if(disbnd->getPrev(0) != disbnd->getNbVertices() -1)
	{
		return -1;
	}
	
	auto start0 = std::chrono::steady_clock::now();
	algorithm::skeletonization::propagation::OptionsSphProp options(alpha);
	std::map<unsigned int,std::list<unsigned int> > pt_assoc_bnd;
	skeleton::GraphSkel2d::Ptr grskelpropag = algorithm::skeletonization::propagation::SpherePropagation2D(disbnd,pt_assoc_bnd,options);
	auto duration0 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start0);
	std::tuple<double,double,int,int> respropag = EvalSkel(dissh,disbnd,grskelpropag);

	if(std::get<1>(respropag) > alpha)
	{
		std::cout << "erreur..." << std::endl;
		return -1;
	}
	
	/*if(!eval)
		std::cout << "Improved propagation skeleton computation: " << duration0.count() << "ms." << std::endl;
	else
	{
		std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
		std::cout.precision(2);
		//std::tuple<double,double,int,int> res0 = EvalSkel(dissh,disbnd,grskelpropag);
		//int t0 = duration0.count();
		//double A0 = std::get<0>(res0); // sym area diff
		//double H0 = std::get<1>(res0); // Hausdorff dist
		//int N0    = std::get<2>(res0); // nb nodes
		//int B0    = std::get<3>(res0); // nb branches
	
		auto startv = std::chrono::steady_clock::now();
		algorithm::skeletonization::propagation::OptionsSphProp options(0.0,0.0);
		skeleton::GraphSkel2d::Ptr grskelvoro = algorithm::skeletonization::propagation::SpherePropagation2D(disbnd,dissh,options);
		auto durationv = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startv);
		std::tuple<double,double,int,int> resv = EvalSkel(dissh,disbnd,grskelvoro);
		int tv = durationv.count(); // Voronoi computation
		double Av = std::get<0>(resv); // sym area diff
		double Hv = std::get<1>(resv); // Hausdorff dist
		int Nv    = std::get<2>(resv); // nb nodes
		int Bv    = std::get<3>(resv); // nb branches

		if(output)
		{
			shape::DiscreteShape<2>::Ptr shpskel(new shape::DiscreteShape<2>(dissh->getWidth(),dissh->getHeight()));
			algorithm::skinning::Filling(shpskel,grskelvoro);
			
			cv::Mat imagerec;
			image.copyTo(imagerec);
			displayopencv::DisplayDiscreteShape(shpskel,imagerec,shpskel->getFrame(),cv::Scalar(0,255,0));
			displayopencv::DisplayDiscreteBoundary(disbnd,imagerec,dissh->getFrame(),cv::Scalar(0,0,0));
			displayopencv::DisplayGraphSkeleton(grskelvoro,imagerec,dissh->getFrame(),cv::Scalar(255,0,0));
			
			cv::imwrite("voro.png", imagerec);
		}

		unsigned int nbPROPAG = 9;//9;
		std::vector<int> t0(nbPROPAG);
		std::vector<double> A0(nbPROPAG);
		std::vector<double> H0(nbPROPAG);
		std::vector<int> N0(nbPROPAG);
		std::vector<int> B0(nbPROPAG);
		for(unsigned int i = 0; i < nbPROPAG; i++)
		{
			double alpha = 0.5 + (double)i*0.5;
			auto start0 = std::chrono::steady_clock::now();
			algorithm::skeletonization::propagation::OptionsSphProp options(alpha,Nv);
			skeleton::GraphSkel2d::Ptr grskelpropag = algorithm::skeletonization::propagation::SpherePropagation2D(disbnd,dissh,options);
			auto duration0 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start0);
			std::tuple<double,double,int,int> res0 = EvalSkel(dissh,disbnd,grskelpropag);
			t0[i] = duration0.count();
			A0[i] = std::get<0>(res0); // sym area diff
			H0[i] = std::get<1>(res0); // Hausdorff dist
			N0[i] = std::get<2>(res0); // nb nodes
			B0[i] = std::get<3>(res0); // nb branches

			if(output)
			{
				shape::DiscreteShape<2>::Ptr shpskel(new shape::DiscreteShape<2>(dissh->getWidth(),dissh->getHeight()));
				algorithm::skinning::Filling(shpskel,grskelpropag);
				
				cv::Mat imagerec;
				image.copyTo(imagerec);
				displayopencv::DisplayDiscreteShape(shpskel,imagerec,shpskel->getFrame(),cv::Scalar(0,255,0));
				displayopencv::DisplayDiscreteBoundary(disbnd,imagerec,dissh->getFrame(),cv::Scalar(0,0,0));
				displayopencv::DisplayGraphSkeleton(grskelpropag,imagerec,dissh->getFrame(),cv::Scalar(255,0,0));
				
				std::ostringstream oss;
				oss.setf( std::ios::fixed, std:: ios::floatfield );
				oss.precision(1);
				oss << "propag_" << alpha << "_" << B0[i] << ".png";
				cv::imwrite(oss.str(), imagerec);
			}
		}

		unsigned int nbSAT = 9;
		std::vector<int> t1(nbSAT);
		std::vector<double> A1(nbSAT);
		std::vector<double> H1(nbSAT);
		std::vector<int> N1(nbSAT);
		std::vector<int> B1(nbSAT);
		for(unsigned int i = 0; i < nbSAT; i++)
		{
			double sat = 1.1 + (double)i*0.1;
			auto start1 = std::chrono::steady_clock::now();
			skeleton::GraphSkel2d::Ptr grskelsat = algorithm::pruning::ScaleAxisTransform(grskelvoro,sat);
			auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start1);
			std::tuple<double,double,int,int> res1 = EvalSkel(dissh,disbnd,grskelsat);
			t1[i] = tv + duration1.count();
			A1[i] = std::get<0>(res1); // sym area diff
			H1[i] = std::get<1>(res1); // Hausdorff dist
			N1[i] = std::get<2>(res1); // nb nodes
			B1[i] = std::get<3>(res1); // nb branches

			if(output)
			{
				shape::DiscreteShape<2>::Ptr shpskel(new shape::DiscreteShape<2>(dissh->getWidth(),dissh->getHeight()));
				algorithm::skinning::Filling(shpskel,grskelsat);
				
				cv::Mat imagerec;
				image.copyTo(imagerec);
				displayopencv::DisplayDiscreteShape(shpskel,imagerec,shpskel->getFrame(),cv::Scalar(0,255,0));
				displayopencv::DisplayDiscreteBoundary(disbnd,imagerec,dissh->getFrame(),cv::Scalar(0,0,0));
				displayopencv::DisplayGraphSkeleton(grskelsat,imagerec,dissh->getFrame(),cv::Scalar(255,0,0));
				
				std::ostringstream oss;
				oss.setf( std::ios::fixed, std:: ios::floatfield );
				oss.precision(1);
				oss << "sat_" << sat << "_" << B1[i] << ".png";
				cv::imwrite(oss.str(), imagerec);
			}
		}

		unsigned int nbLAMBDA = 9;
		std::vector<int> t2(nbLAMBDA);
		std::vector<double> A2(nbLAMBDA);
		std::vector<double> H2(nbLAMBDA);
		std::vector<int> N2(nbLAMBDA);
		std::vector<int> B2(nbLAMBDA);
		for(unsigned int i = 0; i < nbLAMBDA; i++)
		{
			double lambda = 1.0 + (double)i;
			auto start2 = std::chrono::steady_clock::now();
			skeleton::GraphSkel2d::Ptr grskellambda = algorithm::pruning::LambdaMedialAxis(grskelvoro,disbnd,lambda);
			auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start2);
			std::tuple<double,double,int,int> res2 = EvalSkel(dissh,disbnd,grskellambda);
			t2[i] = tv + duration2.count();
			A2[i] = std::get<0>(res2); // sym area diff
			H2[i] = std::get<1>(res2); // Hausdorff dist
			N2[i] = std::get<2>(res2); // nb nodes
			B2[i] = std::get<3>(res2); // nb branches

			if(output)
			{
				shape::DiscreteShape<2>::Ptr shpskel(new shape::DiscreteShape<2>(dissh->getWidth(),dissh->getHeight()));
				algorithm::skinning::Filling(shpskel,grskellambda);
				
				cv::Mat imagerec;
				image.copyTo(imagerec);
				displayopencv::DisplayDiscreteShape(shpskel,imagerec,shpskel->getFrame(),cv::Scalar(0,255,0));
				displayopencv::DisplayDiscreteBoundary(disbnd,imagerec,dissh->getFrame(),cv::Scalar(0,0,0));
				displayopencv::DisplayGraphSkeleton(grskellambda,imagerec,dissh->getFrame(),cv::Scalar(255,0,0));
				
				std::ostringstream oss;
				oss.setf( std::ios::fixed, std:: ios::floatfield );
				oss.precision(0);
				oss << "lambda_" << lambda << "_" << B2[i] << ".png";
				cv::imwrite(oss.str(), imagerec);
			}
		}

		unsigned int nbTHETA = 9;
		std::vector<int> t3(nbTHETA);
		std::vector<double> A3(nbTHETA);
		std::vector<double> H3(nbTHETA);
		std::vector<int> N3(nbTHETA);
		std::vector<int> B3(nbTHETA);
		for(unsigned int i = 0; i < nbTHETA; i++)
		{
			double theta = (double)(i+1)*M_PI/18.0; // every 10 degrees
			auto start3 = std::chrono::steady_clock::now();
			skeleton::GraphSkel2d::Ptr grskeltheta = algorithm::pruning::ThetaMedialAxis(grskelvoro,theta);
			auto duration3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start3);
			std::tuple<double,double,int,int> res3 = EvalSkel(dissh,disbnd,grskeltheta);
			t3[i] = tv + duration3.count();
			A3[i] = std::get<0>(res3); // sym area diff
			H3[i] = std::get<1>(res3); // Hausdorff dist
			N3[i] = std::get<2>(res3); // nb nodes
			B3[i] = std::get<3>(res3); // nb branches

			if(output)
			{
				shape::DiscreteShape<2>::Ptr shpskel(new shape::DiscreteShape<2>(dissh->getWidth(),dissh->getHeight()));
				algorithm::skinning::Filling(shpskel,grskeltheta);
				
				cv::Mat imagerec;
				image.copyTo(imagerec);
				displayopencv::DisplayDiscreteShape(shpskel,imagerec,shpskel->getFrame(),cv::Scalar(0,255,0));
				displayopencv::DisplayDiscreteBoundary(disbnd,imagerec,dissh->getFrame(),cv::Scalar(0,0,0));
				displayopencv::DisplayGraphSkeleton(grskeltheta,imagerec,dissh->getFrame(),cv::Scalar(255,0,0));
				
				std::ostringstream oss;
				oss.setf( std::ios::fixed, std:: ios::floatfield );
				oss.precision(2);
				oss << "theta_" << theta << "_" << B3[i] << ".png";
				cv::imwrite(oss.str(), imagerec);
			}
		}
		
		std::cout << "t ";
		std::cout << tv << " ";
		for(unsigned int i = 0; i < t0.size(); i++)
			std::cout << t0[i] << " ";
		//std::cout << duration0.count() << " ";
		for(unsigned int i = 0; i < t1.size(); i++)
			std::cout << t1[i] << " ";
		for(unsigned int i = 0; i < t2.size(); i++)
			std::cout << t2[i] << " ";
		for(unsigned int i = 0; i < t3.size(); i++)
			std::cout << t3[i] << " ";
		std::cout << std::endl;
		
		std::cout << "A ";
		std::cout << Av << " ";
		for(unsigned int i = 0; i < A0.size(); i++)
			std::cout << A0[i] << " ";
		//std::cout << std::get<0>(respropag) << " ";
		for(unsigned int i = 0; i < A1.size(); i++)
			std::cout << A1[i] << " ";
		for(unsigned int i = 0; i < A2.size(); i++)
			std::cout << A2[i] << " ";
		for(unsigned int i = 0; i < A3.size(); i++)
			std::cout << A3[i] << " ";
		std::cout << std::endl;
		
		std::cout << "H ";
		std::cout << Hv << " ";
		for(unsigned int i = 0; i < H0.size(); i++)
			std::cout << H0[i] << " ";
		//std::cout << std::get<1>(respropag) << " ";
		for(unsigned int i = 0; i < H1.size(); i++)
			std::cout << H1[i] << " ";
		for(unsigned int i = 0; i < H2.size(); i++)
			std::cout << H2[i] << " ";
		for(unsigned int i = 0; i < H3.size(); i++)
			std::cout << H3[i] << " ";
		std::cout << std::endl;
		
		std::cout << "N ";
		std::cout << Nv << " ";
		for(unsigned int i = 0; i < N0.size(); i++)
			std::cout << N0[i] << " ";
		//std::cout << std::get<2>(respropag) << " ";
		for(unsigned int i = 0; i < N1.size(); i++)
			std::cout << N1[i] << " ";
		for(unsigned int i = 0; i < N2.size(); i++)
			std::cout << N2[i] << " ";
		for(unsigned int i = 0; i < N3.size(); i++)
			std::cout << N3[i] << " ";
		std::cout << std::endl;
		
		std::cout << "B ";
		std::cout << Bv << " ";
		for(unsigned int i = 0; i < B0.size(); i++)
			std::cout << B0[i] << " ";
		//std::cout << std::get<3>(respropag) << " ";
		for(unsigned int i = 0; i < B1.size(); i++)
			std::cout << B1[i] << " ";
		for(unsigned int i = 0; i < B2.size(); i++)
			std::cout << B2[i] << " ";
		for(unsigned int i = 0; i < B3.size(); i++)
			std::cout << B3[i] << " ";
		std::cout << std::endl;
	}*/

	if(fileskl.size() != 0)
	{
		fileio::WriteSkeleton2D(grskelpropag,pt_assoc_bnd,fileskl);
	}
	if(filebnd.size() != 0)
		fileio::WriteBoundary2D(disbnd,filebnd);

	shape::DiscreteShape<2>::Ptr shppropag(new shape::DiscreteShape<2>(dissh->getWidth(),dissh->getHeight()));
	algorithm::skinning::Filling(shppropag,grskelpropag);

	cv::Mat imagepropag;
	image.copyTo(imagepropag);
	//displayopencv::DisplayDiscreteShape(shppropag,imagepropag,shppropag->getFrame(),cv::Scalar(0,255,0));
	//displayopencv::DisplayDiscreteBoundary(disbnd,imagepropag,dissh->getFrame(),cv::Scalar(0,0,0));
	displayopencv::DisplayGraphSkeleton(grskelpropag,imagepropag,dissh->getFrame(),cv::Scalar(255,0,0));
	
	if(output)
	{
		std::ostringstream oss;
		oss.setf( std::ios::fixed, std:: ios::floatfield );
		oss.precision(1);
		oss << fileimg << ".png";
		cv::imwrite(oss.str(), imagepropag);
	}

	return 0;
}
