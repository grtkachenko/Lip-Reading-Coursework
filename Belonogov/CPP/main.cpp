#include "pnt.h"
#include "main.h"
const string pathToPython = "/home/vanya/Documents/LR/4/Lip-Reading-Coursework/Belonogov/python/";

const string filmFile = pathToPython + "psychology.mp4";
//const string featureFile = "psychology_f.txt";
const string featureFile = pathToPython + "audio_feat.txt";

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
    MyFrame(): t(0), id(0) {}
    MyFrame(double t, int id, Lips lips): t(t), id(id), lips(lips) {}
};
const int LIPS_SIZE = 20;
const int AUDIO_FEATURE_SIZE = 26;

typedef dlib::matrix<double, AUDIO_FEATURE_SIZE, 1> sample_type;
typedef dlib::radial_basis_kernel<sample_type> kernel_type;

struct Player {
//
//void test() {
//    using namespace cv;
//    Mat img(500, 500, CV_8UC3);
//    RNG &rng = theRNG();
//    for (; ;) {
//        char key;
//        int i, count = (unsigned) rng % 100 + 1;
//        std::vector<Point> points;
//        for (i = 0; i < count; i++) {
//            Point pt;
//            pt.x = rng.uniform(img.cols / 4, img.cols * 3 / 4);
//            pt.y = rng.uniform(img.rows / 4, img.rows * 3 / 4);
//            points.push_back(pt);
//        }
//        std::vector<int> hull;
//        convexHull(Mat(points), hull, true);
//        img = Scalar::all(0);
//        for (i = 0; i < count; i++)
//            circle(img, points[i], 3, Scalar(0, 0, 255), FILLED, LINE_AA);
//        int hullcount = (int) hull.size();
//        Point pt0 = points[hull[hullcount - 1]];
//        for (i = 0; i < hullcount; i++) {
//            Point pt = points[hull[i]];
//            line(img, pt0, pt, Scalar(0, 255, 0), 1, LINE_AA);
//            pt0 = pt;
//        }
//        imshow("hull", img);
//        key = (char) waitKey();
//        if (key == 27 || key == 'q' || key == 'Q') // 'ESC'
//            break;
//    }
//}

    std::vector<pair<double, vector<double> > > audioFeature;
    int n, m;
    double winStep, winLen;

    void readAudioFeature() {
        FILE *feature = fopen(featureFile.data(), "r");
        assert(feature != NULL);
        assert(fscanf(feature, "%d%d%lf%lf", &n, &m, &winStep, &winLen) == 4);
        db2(winStep, winLen);
        //assert(false);
        //db2(n, m);
        for (int i = 0; i < n; i++) {
            double tmr;
            std::vector<double> featureVector(m);
            assert(fscanf(feature, "%lf:", &tmr) == 1);
            for (int j = 0; j < m; j++)
                assert(fscanf(feature, "%lf", &featureVector[j]) == 1);
            audioFeature.push_back(make_pair(tmr, featureVector));
        }
        typedef dlib::matrix < double, 0, 1 > MType;

        dlib::vector_normalizer_pca < MType > normalizer;

        vector < MType > data(n);
        for (int i = 0; i < n; i++) {
            data[i].set_size(m);
            for (int j = 0; j < m; j++)
                data[i](j) = audioFeature[i].sc[j];
        }

        db2(normalizer.in_vector_size(), normalizer.out_vector_size());
        normalizer.train(data);
        db2(normalizer.in_vector_size(), normalizer.out_vector_size());

        //db(audioFeature.size());
        //db("success read");
    }

    void drawRect(cv::Mat &frame, int id) {
        int w = frame.cols;
        int hh = 50;
        int ww = 100;
        cv::Point A(w - ww, 0 + id * hh);
        cv::Point B(w - 1, hh - 1 + id * hh);
        cv::rectangle(frame, A, B, cv::Scalar(0, 0, 0), cv::FILLED, 0);
    }

    void drawLips(cv::Mat &frame, Lips lips, int id) {
        int w = frame.cols;
        int hh = 50;
        int ww = 100;
        cv::Point A(w - ww, 0 + id * hh);
        cv::Point B(w - 1, hh - 1 + id * hh);

        lips.setScale(30);
        lips.shift((A + B) / 2);

        auto segments = lips.getContour();
        for (auto s: segments)
            cv::line(frame, s.fr.getCVPoint(), s.sc.getCVPoint(), cv::Scalar(0, 0, 255));
    }

//void readSubtitle() {
    //assert(freopen("inception.srt", "r", stdin) != 0);
    //std::string s;
    //getline(std::cin, s);
    //SegSet subtitles;
    //while (getline(cin, s)) {
    //if (s.find("-->") != string::npos) {
    //int t1, t2, t3, t4;
    //int r1, r2, r3, r4;
    //assert(sscanf(s.data(), "%d:%d:%d,%d --> %d:%d:%d,%d", &t1, &t2, &t3, &t4, &r1, &r2, &r3, &r4) == 8);
    //double l = t1 * 3600 + t2 * 60 + t3 + t4 / 1000.0;
    //double r = r1 * 3600 + r2 * 60 + r3 + r4 / 1000.0;
    //subtitles.add(l, r);
    //}
    //}
    //cerr << subtitles.getLen() << endl;
//}

    MyFrame prevFrame;


    std::vector<dlib::krls<kernel_type> > lipsPredictor;


    int featureCur = 0;

    void addToLearn(MyFrame curFrame) {
        assert(prevFrame.id + 1 <= curFrame.id);
        if (prevFrame.id + 1 == curFrame.id) {
            double t1 = prevFrame.t;
            double t2 = curFrame.t;
            for (; featureCur < (int) audioFeature.size() && audioFeature[featureCur].fr < t1; featureCur++);

            for (; featureCur < (int) audioFeature.size() && audioFeature[featureCur].fr < t2; featureCur++) {
                std::vector<pnt> upsampl;
                auto l1 = prevFrame.lips.data();
                auto l2 = curFrame.lips.data();
                double tmr = audioFeature[featureCur].fr;
                for (int i = 0; i < (int) l1.size(); i++)
                    upsampl.push_back(l1[i] * (t2 - tmr) / (t2 - t1) + l2[i] * (tmr - t1) / (t2 - t1));
                sample_type m;
                vector<double> g;
                for (auto p: upsampl) {
                    g.push_back(p.x);
                    g.push_back(p.y);
                }
                for (int i = 0; i < (int) audioFeature[featureCur].sc.size(); i++)
                    m(i) = audioFeature[featureCur].sc[i];

                for (int i = 0; i < (int) g.size(); i++)
                    lipsPredictor[i].train(m, g[i]);
            }
        }
        int cntVectors = 0;
        for (int i = 0; i < (int) lipsPredictor.size(); i++)
            cntVectors += lipsPredictor[i].dictionary_size();
        //db(cntVectors);

        prevFrame = curFrame;
    }

    bool static cmpAudio(const pair<double, vector<double> > &a, const pair<double, vector<double> > &b) {
        return a.fr < b.fr;
    }

    Lips getLips(double tmr) {
        int pos = lower_bound(audioFeature.begin(), audioFeature.end(), mp(tmr, vector<double>()), cmpAudio) -
                  audioFeature.begin();
        assert(0 <= pos && pos < (int) audioFeature.size());

        std::vector<pnt> lips;
        sample_type m;
        //db(pos);
        //db(audioFeature.size());
        for (int i = 0; i < AUDIO_FEATURE_SIZE; i++)
            m(i) = audioFeature[pos].sc[i];
        //db2(LIPS_SIZE, lipsPredictor.size());
        for (int i = 0; i < LIPS_SIZE; i++) {
            double x = lipsPredictor[i * 2](m);
            double y = lipsPredictor[i * 2 + 1](m);
            lips.pb(pnt(x, y));
        }

        return Lips(lips);
    }

    double playVideo() {
        //readSubtitle();
        prevFrame = MyFrame(0, -2, Lips());


        readAudioFeature();

        //cv::VideoCapture cap("inception.avi"); // open the default camera
        cv::VideoCapture cap(filmFile.data()); // open the default camera

        if (!cap.isOpened()) { // check if we succeeded
            assert(false);
        }

        //int lastMinute = 3;
        //cap.set(CV_CAP_PROP_POS_MSEC, 1000 * 60 * lastMinute);

        dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
        dlib::shape_predictor pose_model;
        dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;
//        dlib::image_window win;

        std::vector<MyFrame> myFrame;
        Lips sumLips(vector<pnt>(LIPS_SIZE, pnt(0, 0)));
        Average d1, d2, d3;

//    Lips l100;
        int ITER = 500;
        for (int it = 0; it < ITER; it++) {
            cv::Mat frame;
            cap >> frame; // get a new frame from camera
            //double curTime = cap.get(CV_CAP_PROP_POS_MSEC);
            //int frameId = cap.get(CV_CAP_PROP_POS_FRAMES);
            //db2(frameId, it);
            //if (curTime > 1000 * 60 * 3.5) break;
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

            drawRect(frame, 0);
            drawRect(frame, 1);
            drawRect(frame, 2);

            if (shapes.size() == 1) {
                std::vector<pnt> lips;
                for (int i = 48; i < 68; i++) {
                    lips.push_back(pnt(shapes[0].part(i).x(), shapes[0].part(i).y()));
                    pnt C = lips.back();
                    cv::circle(frame, cv::Point(C.x, C.y), 2, cv::Scalar(255, 0, 0), cv::FILLED, cv::LINE_AA);
                }
                int frameId = cap.get(CV_CAP_PROP_POS_FRAMES);
                double tmr = cap.get(CV_CAP_PROP_POS_MSEC) / 1000.0;
                Lips l;
                l = Lips(lips);
                l.normalize();
                //db("draw1");
                drawLips(frame, l, 0); // current lips
                //db("draw2");
                auto l1 = getLips(tmr);
                drawLips(frame, l1, 1); // predicted lips
                double dd = 0;
                if (it >= 2) {
                    dd = lipsDist(l, l1);
                }
                d1.add(dd);

                for (int i = 0; i < (int) sumLips.lips.size(); i++)
                    sumLips.lips[i] = sumLips.lips[i] + l.lips[i];

                auto lipsVec = sumLips.data();
                //db(d2.cnt + 1);
                for (auto &p: lipsVec)
                    p = p / (d2.cnt + 1);

                auto l2 = Lips(lipsVec);
                drawLips(frame, l2, 2); // mean lips
                double dist02 = lipsDist(l2, l);
                d2.add(dist02);
//                cerr << endl;
                //if (it * 2 == ITER)
                    //cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

                if (it < ITER / 2)
                    addToLearn(MyFrame(tmr, frameId, l));
                else {
                    d3.add(dd);
                }
//                if (it < ITER / 2)
//                    cerr << fixed << "\tit: " << it << "    av  dist 0-1  0-2 : " << d1.average() << " " <<
//                    d2.average() << "      cur 0-1 0-2    " << dd << " " << dist02 << endl;
//                else
//                    cerr << fixed << "\tit: " << it << "    av  dist 0-1  0-2 : " << d3.average() << " " <<
//                    d2.average() << "      cur 0-1 0-2    " << dd << " " << dist02 << endl;

            }
//            win.clear_overlay();
//            win.set_image(cimg);
//            win.add_overlay(render_face_detections(shapes));
        }
        return d3.average();
    }
};

int main(int argc, char * argv[]) {
    double alf, beta;
    if (argc == 1) {
        alf = 0.01;
        beta = 0.001;
    }
    else if (argc == 3) {
        int r1, r2;
        sscanf(argv[1], "%d", &r1);
        sscanf(argv[2], "%d", &r2);
        alf = 1.0 / r1;
        beta = 1.0 / r2;
    }
    else
        assert(false);
    //db2(alf, beta);
//    FILE * config = fopen("config.txt", "r");
//    int vectorCnt;
//    assert(fscanf(config, "%d", &vectorCnt) == 1);
//    vector < pair < int, double > > answers;
//    for (int i = 2; i < 20; i++) {
    Player player;
    player.lipsPredictor.assign(LIPS_SIZE * 2, dlib::krls<kernel_type>(kernel_type(alf), beta, 20));
    double res = player.playVideo();
//    answers.pb(mp(i, res));
    printf("%lf\n", res); //    }
//    for (auto x: answers)
//        db2(x.fr, x.sc);
    return 0;
}

