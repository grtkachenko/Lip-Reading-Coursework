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
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>
#include <utility>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

using namespace dlib;
using namespace std;

#define pb push_back
#define mp make_pair
#define frs first
#define snd second

typedef pair <int, int> pii;
typedef pair <double, double> pdd;

std::set <pdd> subtitles;

void parse() {
    string s;

    while (getline(cin, s)) {
        if (s.find(" --> ") == string::npos) {
            continue;
        }

        int a, b, c, d;
        int e, f, g, h;
        double time1, time2;

        sscanf(s.c_str(), "%d:%d:%d,%d --> %d:%d:%d,%d", &a, &b, &c, &d, &e, &f, &g, &h);
        time1 = a * 3600.0 + b * 60.0 + c + d / 1000.0;
        time2 = e * 3600.0 + f * 60.0 + g + h / 1000.0;
        subtitles.insert(mp(time1, time2));
    }
}

double get_time(std::set <pdd> a, double start, double finish) {
    double ans = 0.0;

    for (std::set <pdd> :: iterator it = a.begin(); it != a.end(); it++) {
        pdd curr = *it;

        if (curr.snd < start) {
            continue;
        }

        if (curr.frs > finish) {
            break;
        }

        if (curr.frs < start) {
            ans += (min(curr.snd, finish) - start);
            continue;
        }

        if (curr.snd > finish) {
            ans += (finish - curr.frs);
            continue;
        }

        ans += (curr.snd - curr.frs);
    }

    return ans;
}

double unite(std::set <pdd> a, std::set <pdd> b) {
    std::vector < pair <double, int> > segments;

    for (std::set <pdd> :: iterator it = a.begin(); it != a.end(); it++) {
        pdd curr = *it;
        segments.pb(mp(curr.frs, 1));
        segments.pb(mp(curr.snd, -1));
    }

    for (std::set <pdd> :: iterator it = b.begin(); it != b.end(); it++) {
        pdd curr = *it;
        segments.pb(mp(curr.frs, 1));
        segments.pb(mp(curr.snd, -1));
    }

    sort(segments.begin(), segments.end());
    int balance = 0;
    std::set <pdd> all;
    double prev = 0.0;

    for (int i = 0; i < (int) segments.size(); i++) {
        pair <double, int> curr = segments[i];
        balance += curr.snd;

        if (curr.snd == 1 && balance == 2) {
            prev = curr.frs;
        }

        if (curr.snd == -1 && balance == 1) {
            all.insert(mp(prev, curr.frs));
        }
    }

    double ans = 0.0;

    for (std::set <pdd> :: iterator it = all.begin(); it != all.end(); it++) {
        pdd curr = *it;
        ans += (curr.snd - curr.frs);
    }

    return ans;
}

int main() {
    try {
        freopen("test.srt", "r", stdin);
        parse();
        fclose(stdin);
        cv::VideoCapture cap("test.mp4");
        cap.set(CV_CAP_PROP_POS_MSEC, 2900);
        std::set <pdd> speech;
        double prev = 2.9;
        double prev_next = 2.9;
        double next = 5;
        int step = 5;
        image_window win;

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

        // Grab and process frames until the main window is closed by the user.
        while(!win.is_closed()) {
            // Grab a frame
            double curr = cap.get(CV_CAP_PROP_POS_MSEC) / 1000.0;
            cv::Mat temp;
            cap >> temp;
            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            cv_image<bgr_pixel> cimg(temp);

            // Detect faces 
            std::vector<rectangle> faces = detector(cimg);
            // Find the pose of each face.
            std::vector<full_object_detection> shapes;
            for (unsigned long i = 0; i < faces.size(); ++i) {
                shapes.push_back(pose_model(cimg, faces[i]));
            }

            if (shapes.size() != 1) {
                continue;
            }

            point up = shapes[0].part(62);
            point down = shapes[0].part(66);
            point left = shapes[0].part(60);
            point right = shapes[0].part(64);
            //cerr << "Vert: " << (down - up).length() << endl;
            //cerr << "Hor: " << (right - left).length() << endl;
            double vert = (down - up).length();
            double hor = (right - left).length();

            if (vert > 2.5 || hor < 50.0) {
                circle(temp, cv::Point(20, 20), 10, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_AA);
                speech.insert(mp(prev, curr));
            }

            // Display it all on the screen
            win.clear_overlay();
            win.set_image(cimg);
            win.add_overlay(render_face_detections(shapes));
            prev = curr;

            if (cv::waitKey(1) >= 0) {
                break;
            }

            if (curr >= next) {
                double subtitles_time = get_time(subtitles, prev_next, curr);
                double speech_time = get_time(speech, prev_next - 0.1, curr + 0.1);
                double bingo = unite(subtitles, speech);
                printf("From %0.3lf to %0.3lf seconds\n", prev_next, curr);
                printf("Subtitles: %0.3lf seconds\n", subtitles_time);
                printf("Speech: %0.3lf seconds\n", speech_time);
                printf("Union: %0.3lf seconds\n", bingo);
                printf("Match: %0.2lf %%\n", bingo / max (subtitles_time, speech_time) * 100.0);
                speech.clear();
                prev_next = curr;
                next += step;
            }
        }
    }
    catch(serialization_error& e) {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl << e.what() << endl;
    }
    catch(exception& e) {
        cout << e.what() << endl;
    }
}

