// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.  
    

    This example is essentially just a version of the face_landmark_detection_ex.cpp
    example modified to use OpenCV's VideoCapture object to read from a camera instead 
    of files.


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.  
*/

#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

//using namespace cv;


#define db(x) cerr << #x << " = " << x << endl
#define db2(x, y) cerr << "(" << #x << ", " << #y << ") = (" << x << ", " << y << ")\n"

using namespace dlib;
using namespace std;


void test() {
    using namespace cv;
    Mat img(500, 500, CV_8UC3);
    RNG& rng = theRNG();
    for(;;)
    {
        char key;
        int i, count = (unsigned)rng%100 + 1;
        std::vector<Point> points;
        for( i = 0; i < count; i++ )
        {
            Point pt;
            pt.x = rng.uniform(img.cols/4, img.cols*3/4);
            pt.y = rng.uniform(img.rows/4, img.rows*3/4);
            points.push_back(pt);
        }
        std::vector<int> hull;
        convexHull(Mat(points), hull, true);
        img = Scalar::all(0);
        for( i = 0; i < count; i++ )
            circle(img, points[i], 3, Scalar(0, 0, 255), FILLED, LINE_AA);
        int hullcount = (int)hull.size();
        Point pt0 = points[hull[hullcount-1]];
        for( i = 0; i < hullcount; i++ )
        {
            Point pt = points[hull[i]];
            line(img, pt0, pt, Scalar(0, 255, 0), 1,LINE_AA);
            pt0 = pt;
        }
        imshow("hull", img);
        key = (char)waitKey();
        if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
            break;
    }
}

int main()
{
    //test();

    try
    {
        cv::VideoCapture cap(0);
        image_window win;

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

        // Grab and process frames until the main window is closed by the user.

        while(!win.is_closed()) {
            // Grab a frame
            cv::Mat temp;
            cap >> temp;
            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            //line(temp, cv::Point( 15, 20 ), cv::Point( 70, 50), cv::Scalar( 110, 220, 0 ),  2, 8 );
            cv_image<bgr_pixel> cimg(temp);
            //line(temp, cv::Point( 25, 20 ), cv::Point( 80, 50), cv::Scalar( 110, 220, 0 ),  2, 8 );


            //win.clear_overlay();
            //win.set_image(cimg);

            //  continue;
            //while (true);

            // Detect faces 
            std::vector<rectangle> faces = detector(cimg);
            //db(faces.size());
            //if (!faces.empty()) {
                //db2(faces[0].bottom(), faces[0].top());
                //db2(faces[0].left(), faces[0].right());
            //}

            // Find the pose of each face.
            std::vector<full_object_detection> shapes;
            for (unsigned long i = 0; i < faces.size(); ++i)
                shapes.push_back(pose_model(cimg, faces[i]));


            ///48 ... 68 - lips
            // 60 ... 68 - inner part
            // 66 && 62 - up down;
            // 60 && 64 left right


            if (!shapes.empty()) {
                //db(shapes[0].num_parts());
                assert(shapes[0].num_parts() == 68);
                //for (int i = 48; i < shapes[0].num_parts(); i++)  {
                for (int i = 60; i < 68; i += 2)  {
                    //if (i != 64 && i != 60 && ) continue;
                    point A = shapes[0].part(i);
                    //db2(A.x(), A.y());
                    circle(temp, cv::Point(A.x(), A.y()), 3, cv::Scalar(0, 0, (int)(255 * i * 1.0 / shapes[0].num_parts())), cv::FILLED, cv::LINE_AA);
                }
                double leftToRigth = (shapes[0].part(60) - shapes[0].part(64)).length();
                double upToDown = (shapes[0].part(62) - shapes[0].part(66)).length();

                if (upToDown / leftToRigth > 0.15)
                    circle(temp, cv::Point(20, 20), 10, cv::Scalar(255, 0, 0), cv::FILLED, cv::LINE_AA);
            }

            win.clear_overlay();
            win.set_image(cimg);
            win.add_overlay(render_face_detections(shapes));
        }
    }
    catch(serialization_error& e)
    {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl << e.what() << endl;
    }
    catch(exception& e)
    {
        cout << e.what() << endl;
    }
}

