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
#include "opencv2/opencv.hpp"
#include "SegSet.h"
#include <vector>
#include <chrono>
#include <stdio.h>

//using namespace cv;



#define db(x) cerr << #x << " = " << x << endl
#define db2(x, y) cerr << "(" << #x << ", " << #y << ") = (" << x << ", " << y << ")\n"
#define equal equall
#define less lesss

using namespace dlib;

bool equal(double a, double b) {
    return abs(a - b) < 1e-9;
}

bool less(double a, double b) {
    return a < b && !equal(a, b);
}

bool lessE(double a, double b) {
    return a < b || equal(a, b);
}

struct pnt {
    double x, y;
    pnt () {}
    pnt (double x, double y): x(x), y(y) {}
    pnt operator + (pnt A) {
        return pnt(x + A.x, y + A.y);
    }
    pnt operator - (pnt A) {
        return pnt(x - A.x, y - A.y);
    }
    pnt operator * (double k) {
        return pnt(x * k, y * k);
    }
    pnt operator / (double k) {
        return pnt(x / k, y / k);
    }
    double len() {
        return sqrt(x * x + y * y);
    }
    pnt rotate(double ang) {
        return pnt(x * cos(ang) - y * sin(ang), x * sin(ang) + y * cos(ang));
    }
};

//
//struct Lips {
//    vector < point > data;
//    Lips(int size) {
//        data.resize(size);
//    }
//};


struct MyFrame {
    double time;
    int id;
    std::vector < pnt > lips;
    MyFrame(double time, int id, std::vector < pnt > lips): time(time), id(id), lips(lips) {}
};


void test() {
    using namespace cv;
    Mat img(500, 500, CV_8UC3);
    RNG &rng = theRNG();
    for (; ;) {
        char key;
        int i, count = (unsigned) rng % 100 + 1;
        std::vector<Point> points;
        for (i = 0; i < count; i++) {
            Point pt;
            pt.x = rng.uniform(img.cols / 4, img.cols * 3 / 4);
            pt.y = rng.uniform(img.rows / 4, img.rows * 3 / 4);
            points.push_back(pt);
        }
        std::vector<int> hull;
        convexHull(Mat(points), hull, true);
        img = Scalar::all(0);
        for (i = 0; i < count; i++)
            circle(img, points[i], 3, Scalar(0, 0, 255), FILLED, LINE_AA);
        int hullcount = (int) hull.size();
        Point pt0 = points[hull[hullcount - 1]];
        for (i = 0; i < hullcount; i++) {
            Point pt = points[hull[i]];
            line(img, pt0, pt, Scalar(0, 255, 0), 1, LINE_AA);
            pt0 = pt;
        }
        imshow("hull", img);
        key = (char) waitKey();
        if (key == 27 || key == 'q' || key == 'Q') // 'ESC'
            break;
    }
}

std::vector < pair < double, std::vector < double > > > audioFeature;
int n, m;
double winStep, winLen;

void readAudioFeature() {
    FILE * feature = fopen("audio_feat.txt", "r");
    assert(fscanf(feature, "%d%d%lf%lf", &n, &m, &winStep, &winLen) == 4);
    for (int i = 0; i < n; i++) {
        double tmr;
        std::vector < double > featureVector(m);
        assert(fscanf(feature, "%lf:", &tmr) == 1);
        for (int j = 0; j < m; j++)
            assert(fscanf(feature, "%lf", &featureVector[j]) == 1);
        audioFeature.push_back(make_pair(tmr, featureVector));
    }
    db("success read");
}

void playVideo() {
    db("start function");
    assert(freopen("inception.srt", "r", stdin) != 0);

    string s;
    int cnt;
    db("start loop");
    getline(cin, s);
    db(s);
    SegSet subtitles;
    while (getline(cin, s)) {
        if (s.find("-->") != string::npos) {
            cnt++;
            //db(cnt);
            int t1, t2, t3, t4;
            int r1, r2, r3, r4;
            assert(sscanf(s.data(), "%d:%d:%d,%d --> %d:%d:%d,%d", &t1, &t2, &t3, &t4, &r1, &r2, &r3, &r4) == 8);
//            if (res != 8) {
//                db2(res, cnt);
//                cerr << s << endl;
//            }
//            assert(res == 8);
            //cerr << "t1 t2 t3 t4 " << t1 << " " << t2 << " " << t3 << " " << t4 << endl;
            double l = t1 * 3600 + t2 * 60 + t3 + t4 / 1000.0;
            double r = r1 * 3600 + r2 * 60 + r3 + r4 / 1000.0;
            //cerr << "t1 t2 t3 t4 " << t1 << " " << t2 << " " << t3 << " " << t4 << endl;
            subtitles.add(l, r);
        }
    }
    //subtitles.print();
    cerr << subtitles.getLen() << endl;
    db(cnt);
    //exit(0);

    using namespace cv;

    VideoCapture cap("inception.avi"); // open the default camera
    //VideoCapture cap(0); // open the default camera


    if (!cap.isOpened()) { // check if we succeeded
        assert(false);
    }
    int lastMinute = 3;
    cap.set(CV_CAP_PROP_POS_MSEC, 1000 * 60 * lastMinute);
    //cap.set(CV_CAP_PROP_POS_MSEC, 1000 * 470);

    frontal_face_detector detector = get_frontal_face_detector();
    shape_predictor pose_model;
    deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

    image_window win;


    SegSet oneFace;
    double prevFrame = lastMinute * 60;
    std::vector < MyFrame > myFrame;
    for (int it = 0; ;it++) { // TODO change restriction
        Mat frame;
        cap >> frame; // get a new frame from camera
        double curTime = cap.get(CV_CAP_PROP_POS_MSEC);
        if (curTime > 1000 * 60 * 3.5) break;

        cv_image<bgr_pixel> cimg(frame);
        std::vector<dlib::rectangle> faces = detector(cimg);
        std::vector<full_object_detection> shapes;
        for (unsigned long i = 0; i < faces.size(); ++i)
            shapes.push_back(pose_model(cimg, faces[i]));

        ///48 ... 68 - lips
        // 60 ... 68 - inner part
        // 66 && 62 - up down;
        // 60 && 64 left right

        double tmr = cap.get(CV_CAP_PROP_POS_MSEC) / 1000.0;

        if (shapes.size() == 1) {
            std::vector< pnt > lips;
            for (int i = 48; i < 68; i++) {
                lips.push_back(pnt(shapes[0].part(i).x(), shapes[0].part(i).y()));
                //point C = lips.back();
                //circle(frame, cv::Point(C.x(), C.y()), 2, cv::Scalar(255, 0, 0), cv::FILLED, cv::LINE_AA);
            }
            pnt Mid = (lips[0] + lips[6]) / 2;
            for (auto &p: lips)
                p = p - Mid;
            double len = (lips[0] - lips[6]).len() / 2;
            assert(equal(lips[0].len(), lips[6].len()));
            for (auto &p: lips)
                p = p / len;
            double ang = atan2(lips[6].y, lips[6].x);
            for (auto &p: lips)
                p = p.rotate(-ang);
            assert(equal(0, atan2(lips[6].y, lips[6].x)));

            myFrame.push_back(MyFrame(tmr, it, lips));
        }
        win.clear_overlay();
        win.set_image(cimg);
        win.add_overlay(render_face_detections(shapes));
    }

    for (int i = 0; i < (int)myFrame.size(); ) {
        int j = i;
        for (; i < (int)myFrame.size() && i - myFrame[i].id == j - myFrame[j].id; i++);
        if (i - j > 1) {
            oneFace.add(myFrame[j].time, myFrame[i - 1].time);
        }
    }

    SegSet faceAndSub = oneFace & subtitles;
    db(faceAndSub.getLen());


    readAudioFeature();

    std::vector < pair < std::vector < double >, std::vector < pnt > > > dataForLearning;
    int cur = 0;
    int curMyFrame = 0;
    for (int i = 0; i < (int)audioFeature.size(); i++) {
        double tmr = audioFeature[i].first + winLen / 2;
        for (; cur < (int)faceAndSub.data.size() && faceAndSub.data[cur].sc < tmr; cur++);
        if (cur < (int)faceAndSub.data.size() && faceAndSub.data[cur].fr <= tmr && tmr <= faceAndSub.data[cur].sc) {
            for (; curMyFrame < (int)myFrame.size() && less(myFrame[curMyFrame].time, tmr); curMyFrame++);
            assert(curMyFrame < (int)myFrame.size());
            auto & m1 = myFrame[curMyFrame - 1];
            auto & m2 = myFrame[curMyFrame];
            assert(m1.id + 1 == m2.id);
            double len = m2.time - m1.time;
            double l = tmr - m1.time;
            double r = m2.time - tmr;
            std::vector < pnt > lips;
            for (int j = 0; j < (int)m1.lips.size(); j++)
                lips.push_back((m1.lips[j] * r + m2.lips[j] * l) / len);
            dataForLearning.push_back(make_pair(audioFeature[i].second, lips));
        }
    }
    db(dataForLearning.size());



        /// old statistic
//        if (shapes.size() == 1) {
//            oneFace.add(prevFrame, tmr);
//        }
//
//
//        if (!shapes.empty()) {
//            assert(shapes[0].num_parts() == 68);
//            for (int i = 60; i < 68; i += 2) {
//                point A = shapes[0].part(i);
//                circle(frame, cv::Point(A.x(), A.y()), 3,
//                       cv::Scalar(0, 0, (int) (255 * i * 1.0 / shapes[0].num_parts())), cv::FILLED, cv::LINE_AA);
//            }
//            double leftToRigth = (shapes[0].part(60) - shapes[0].part(64)).length();
//            double upToDown = (shapes[0].part(62) - shapes[0].part(66)).length();
//
//            if (upToDown / leftToRigth > 0.07) {
//                circle(frame, cv::Point(20, 20), 10, cv::Scalar(255, 0, 0), cv::FILLED, cv::LINE_AA);
//                speak.add(prevFrame, tmr);
//            }
//
//        }
//
//        win.clear_overlay();
//        win.set_image(cimg);
//        win.add_overlay(render_face_detections(shapes));
//        if (waitKey(30) >= 0) break;
//        //if (it % 400 == 0)
//            //cout << tmr << endl;
//
//
//        if (tmr / 60 > (lastMinute + 1)) {
//            SegSet M;
//            M.add(lastMinute * 60, (lastMinute + 1) * 60);
//            SegSet A = subtitles & oneFace;
//            SegSet B = speak & oneFace;
//            ///(M & subtitles).print();
//            //oneFace.print();
//            double lenA = A.getLen();
//            double lenB = B.getLen();
//            double together = (A & B).getLen();
//
//            cerr.precision(2);
//            cerr << fixed;
//            cerr << "oneFace : " << oneFace.getLen() << "   sub speak  together: " << lenA << " " << lenB << " " << together << "    proportion % " << together / max(lenA, lenB) <<  endl;
//
//
//            oneFace.clear();
//            speak.clear();
//            lastMinute++;
//        }
//        prevFrame = tmr;
//    }
//
//
//
//    exit(0);
}

int main() {
    playVideo();
//
//    try {
//        cv::VideoCapture cap(0);
//        image_window win;
//
//        // Load face detection and pose estimation models.
//        frontal_face_detector detector = get_frontal_face_detector();
//        shape_predictor pose_model;
//        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;
//
//        // Grab and process frames until the main window is closed by the user.
//
//        while (!win.is_closed()) {
//            // Grab a frame
//            cv::Mat temp;
//            cap >> temp;
//            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
//            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
//            // long as temp is valid.  Also don't do anything to temp that would cause it
//            // to reallocate the memory which stores the image as that will make cimg
//            // contain dangling pointers.  This basically means you shouldn't modify temp
//            // while using cimg.
//            //line(temp, cv::Point( 15, 20 ), cv::Point( 70, 50), cv::Scalar( 110, 220, 0 ),  2, 8 );
//            cv_image<bgr_pixel> cimg(temp);
//            //line(temp, cv::Point( 25, 20 ), cv::Point( 80, 50), cv::Scalar( 110, 220, 0 ),  2, 8 );
//
//
//            //win.clear_overlay();
//            //win.set_image(cimg);
//
//            //  continue;
//            //while (true);
//
//            // Detect faces
//            std::vector<rectangle> faces = detector(cimg);
//            //db(faces.size());
//            //if (!faces.empty()) {
//            //db2(faces[0].bottom(), faces[0].top());
//            //db2(faces[0].left(), faces[0].right());
//            //}
//
//            // Find the pose of each face.
//            std::vector<full_object_detection> shapes;
//            for (unsigned long i = 0; i < faces.size(); ++i)
//                shapes.push_back(pose_model(cimg, faces[i]));
//
//
//            ///48 ... 68 - lips
//            // 60 ... 68 - inner part
//            // 66 && 62 - up down;
//            // 60 && 64 left right
//
//
//            if (!shapes.empty()) {
//                //db(shapes[0].num_parts());
//                assert(shapes[0].num_parts() == 68);
//                //for (int i = 48; i < shapes[0].num_parts(); i++)  {
//                for (int i = 60; i < 68; i += 2) {
//                    //if (i != 64 && i != 60 && ) continue;
//                    point A = shapes[0].part(i);
//                    //db2(A.x(), A.y());
//                    circle(temp, cv::Point(A.x(), A.y()), 3,
//                           cv::Scalar(0, 0, (int) (255 * i * 1.0 / shapes[0].num_parts())), cv::FILLED,
//                           cv::LINE_AA);
//                }
//                double leftToRigth = (shapes[0].part(60) - shapes[0].part(64)).length();
//                double upToDown = (shapes[0].part(62) - shapes[0].part(66)).length();
//
//                if (upToDown / leftToRigth > 0.15)
//                    circle(temp, cv::Point(20, 20), 10, cv::Scalar(255, 0, 0), cv::FILLED, cv::LINE_AA);
//            }
//
//            win.clear_overlay();
//            win.set_image(cimg);
//            win.add_overlay(render_face_detections(shapes));
//        }
//    }
//    catch (serialization_error &e) {
//        cout << "You need dlib's default face landmarking model file to run this example." << endl;
//        cout << "You can get it from the following URL: " << endl;
//        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
//        cout << endl << e.what() << endl;
//    }
//    catch (exception &e) {
//        cout << e.what() << endl;
//    }
}

