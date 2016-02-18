#include "SegSet.h"
#include "pnt.h"
#include "main.h"



bool equal(double a, double b) {
    return abs(a - b) < 1e-9;
}

bool less(double a, double b) {
    return a < b && !equal(a, b);
}

bool lessE(double a, double b) {
    return a < b || equal(a, b);
}


struct MyFrame {
    double t;
    int id;
    Lips lips;
    MyFrame(double t, int id, Lips lips): t(t), id(id), lips(lips) {}
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

void drawRect(cv::Mat & frame) {
    int w = frame.cols;
    int hh = 50;
    int ww = 100;
    cv::Point A(w - ww, 0);
    cv::Point B(w - 1, hh - 1);
    cv::rectangle(frame, A, B, cv::Scalar(0, 0, 0), cv::FILLED, 0);
}

void drawLips(cv::Mat & frame, Lips lips) {
    int w = frame.cols;
    int hh = 50;
    int ww = 100;
    cv::Point A(w - ww, 0);
    cv::Point B(w - 1, hh - 1);

    lips.setScale(30);  
    lips.shift((A + B) / 2);

    auto segments = lips.getContour();
    for (auto s: segments)
        cv::line(frame, s.fr.getCVPoint(), s.sc.getCVPoint(), cv::Scalar(0, 0, 255)); 
}

void readSubtitle() {
    assert(freopen("inception.srt", "r", stdin) != 0);
    string s;
    getline(cin, s);
    SegSet subtitles;
    while (getline(cin, s)) {
        if (s.find("-->") != string::npos) {
            int t1, t2, t3, t4;
            int r1, r2, r3, r4;
            assert(sscanf(s.data(), "%d:%d:%d,%d --> %d:%d:%d,%d", &t1, &t2, &t3, &t4, &r1, &r2, &r3, &r4) == 8);
            double l = t1 * 3600 + t2 * 60 + t3 + t4 / 1000.0;
            double r = r1 * 3600 + r2 * 60 + r3 + r4 / 1000.0;
            subtitles.add(l, r);
        }
    }
    cerr << subtitles.getLen() << endl;
}

MyFrame prevFrame(0, -2, Lips());
    

void addToLearn(MyFrame curFrame) {
    assert(prevFrame.id + 1 <= curFrame.id);
    if (prevFrame.id + 1 == curFrame.id) {
        

    }
    prevFrame = curFrame;
}

void playVideo() {
    //readSubtitle();
    
    cv::VideoCapture cap("inception.avi"); // open the default camera

    if (!cap.isOpened()) { // check if we succeeded
        assert(false);
    }

    int lastMinute = 3;
    cap.set(CV_CAP_PROP_POS_MSEC, 1000 * 60 * lastMinute);

    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor pose_model;

    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

    dlib::image_window win;

    SegSet oneFace;
    std::vector < MyFrame > myFrame;
    for (int it = 0; ;it++) { // TODO change restriction
        cv::Mat frame;
        cap >> frame; // get a new frame from camera
        double curTime = cap.get(CV_CAP_PROP_POS_MSEC);
        db(it);
        if (curTime > 1000 * 60 * 3.5) break;

        dlib::cv_image<dlib::bgr_pixel> cimg(frame);
        std::vector<dlib::rectangle> faces = detector(cimg);
        std::vector<dlib::full_object_detection> shapes;
        for (unsigned long i = 0; i < faces.size(); ++i)
            shapes.push_back(pose_model(cimg, faces[i]));

        ///48 ... 68 - lips
        // 60 ... 68 - inner part
        // 66 && 62 - up down;
        // 60 && 64 left right


        //cv::Point A(w - ww, 0);
        //cv::Point B(w - 1, hh);
        //db2(A.x, A.y);
        //db2(B.x, B.y);

        drawRect(frame);
        if (shapes.size() == 1) {
            std::vector < pnt > lips;
            for (int i = 48; i < 68; i++) {
                lips.push_back(pnt(shapes[0].part(i).x(), shapes[0].part(i).y()));
                pnt C = lips.back();
                cv::circle(frame, cv::Point(C.x, C.y), 2, cv::Scalar(255, 0, 0), cv::FILLED, cv::LINE_AA);
            }
            int frameId = cap.get(CV_CAP_PROP_POS_FRAMES);
            double tmr = cap.get(CV_CAP_PROP_POS_MSEC) / 1000.0;
            Lips l(lips);
            l.normalize();
            addToLearn(MyFrame(tmr, frameId, l));
            drawLips(frame, l);
            //myFrame.push_back(MyFrame(tmr, it, l));
        }
        win.clear_overlay();
        win.set_image(cimg);
        win.add_overlay(render_face_detections(shapes));
    }




    //for (int i = 0; i < (int)myFrame.size(); ) {
        //int j = i;
        //for (; i < (int)myFrame.size() && i - myFrame[i].id == j - myFrame[j].id; i++);
        //if (i - j > 1) {
            //oneFace.add(myFrame[j].time, myFrame[i - 1].time);
        //}
    //}

    //SegSet faceAndSub = oneFace & subtitles;
    //db(faceAndSub.getLen());


    //readAudioFeature();

    //std::vector < pair < std::vector < double >,  Lips > > dataForLearning;
    //int cur = 0;
    //int curMyFrame = 0;
    //for (int i = 0; i < (int)audioFeature.size(); i++) {
        //double tmr = audioFeature[i].first + winLen / 2;
        //for (; cur < (int)faceAndSub.data.size() && faceAndSub.data[cur].sc < tmr; cur++);
        //if (cur < (int)faceAndSub.data.size() && faceAndSub.data[cur].fr <= tmr && tmr <= faceAndSub.data[cur].sc) {
            //for (; curMyFrame < (int)myFrame.size() && less(myFrame[curMyFrame].time, tmr); curMyFrame++);
            //assert(curMyFrame < (int)myFrame.size());
            //auto & m1 = myFrame[curMyFrame - 1];
            //auto & m2 = myFrame[curMyFrame];
            //assert(m1.id + 1 == m2.id);
            //double len = m2.time - m1.time;
            //double l = tmr - m1.time;
            //double r = m2.time - tmr;
            //std::vector < pnt > lips;
            //auto lipsPrev = m1.lips.getData();
            //auto lipsNext = m2.lips.getData();

            //for (int j = 0; j < (int)lipsPrev.size(); j++)
                //lips.push_back((lipsPrev[j] * r + lipsNext[j] * l) / len);
            //dataForLearning.push_back(make_pair(audioFeature[i].second, Lips(lips)));
        //}
    //}
    //db(dataForLearning.size());




}

int main() {
    playVideo();

}

