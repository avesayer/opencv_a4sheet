
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
//using namespace std;

struct CameraPose {
    Point3f position;
    Point3f orientation;
    
    friend std::ostream &operator<<(std::ostream &os, const CameraPose &cam);
};

std::ostream &operator<<(std::ostream &os, const CameraPose &cam) {
    
    std::cout << "position: " << cam.position << "\n";
    std::cout << "orientation: " << cam.orientation << "\n";
    
    return os;
}

class ListDetector {
    public:
    ListDetector() {}
    
    std::vector<Point> findSheetContour(const Mat &img) {
        
        // Return either one vector with 4 points or empty vector
        std::vector<Point> result;
        result.reserve(4);
        
        // Convert to gray color
        Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);
        
        // Apply gaussian blue with 11x11 kernel
        GaussianBlur(gray, gray, Size(11, 11), 0);
        
        // Dilate the image with 9x9 kernel
        Mat kernel = getStructuringElement(MORPH_RECT, Size(9, 9));
        dilate(gray, gray, kernel);
        
        // Apply Canny for edge detection
        Canny(gray, gray, 0, cannyThreshold, apertureSize);
        
        std::vector<std::vector<Point>> conts;
        findContours(gray, conts, RETR_LIST, CHAIN_APPROX_SIMPLE);
        
        std::vector<Point> approx;
        for(size_t i = 0; i < conts.size(); ++i) {
            
            // Already found one target
            if (result.size() > 0) break;
            
            // Approximate contour with accuracy proportional
            // to the contour perimeter
            approxPolyDP(conts[i], approx, arcLength(conts[i], true)*0.02, true);
            
            if (approx.size() == 4 && fabs(contourArea(approx)) > 1000.0f && isContourConvex(approx))
            {
                double maxCosine = 0;
                
                for( int j = 2; j < 5; j++ )
                {
                    // Find the maximum cosine of the angle between joint edges
                    double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                    maxCosine = MAX(maxCosine, cosine);
                }
                
                // If cosines of all angles are small
                // (all angles are ~90 degree) then write quandrange
                // vertices to resultant sequence
                if( maxCosine < 0.3) 
                    result = approx;
            }
        }
        
        return (result);
    }
    
    // TODO: Implementation required
    CameraPose findCameraPose(const Mat& img, const std::vector<Point> &quad) {
        CameraPose pos = {};
        return (pos);
    }
    
    private:
    int cannyThreshold = 50, apertureSize = 3;
    
    double angle( Point pt1, Point pt2, Point pt0 ) {
        double dx1 = pt1.x - pt0.x;
        double dy1 = pt1.y - pt0.y;
        double dx2 = pt2.x - pt0.x;
        double dy2 = pt2.y - pt0.y;
        return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
    }
    
};

int main(int argc, char** argv)
{
    const String keys =
        "{help h       || print this message }"
        "{path p       || path to input image }"
        "{show s       |false| flag for showing result images }";
    
    CommandLineParser parser(argc, argv, keys);
    
    std::vector<cv::String> paths;
    
    std::string img_path = parser.get<std::string>("path");
    bool show_img = parser.get<bool>("show");
    
    if (parser.has("help"))  {
        parser.printMessage();
        return 0;
    }
    
    if (!img_path.empty()) {
        paths.push_back(img_path);
    }
    else {
        glob("*.jpg", paths, false);
    }
    
    ListDetector detector;
    
    for (int i = 0; i < paths.size(); ++i) {
        
        cv::String img_path = paths[i];
        
        Mat src = imread(img_path, IMREAD_COLOR); // Load an image
        if (src.empty())
        {
            std::cout << "Could not open or find the image: " << img_path << "\n";
            continue;
        }
        
        std::vector<Point> result_quad = detector.findSheetContour(src);
        
        CameraPose pose = detector.findCameraPose(src, result_quad);
        std::cout << img_path << ": \n";
        std::cout << pose;
        
        if (show_img) {
            Scalar color(255, 0, 0);
            polylines(src, result_quad, true, color, 3);
            
            imshow(img_path, src);
        }
    }
    
    if (show_img)
        waitKey(0);
    
    return 0;
}