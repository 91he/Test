#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

using namespace cv;
using namespace std;
using namespace xfeatures2d;

/*
 * stitcher:
 * FeatureDetector_create
 * DescriptorExtractor_create
 * DescriptorMatcher_create
 * findHomography
 * warpPerspective
 *
 * scanner:
 * findHomography
 * computeIntersect
 * getPerspectiveTransform
 */

int main(int argc, char **argv){
    Mat img1, img2, gray1, gray2, dst1, dst2;

    //initModule_features2d();
    img1 = imread(argv[2], IMREAD_COLOR);
    img2 = imread(argv[1], IMREAD_COLOR);
    cvtColor(img1, gray1, CV_BGR2GRAY);
    cvtColor(img2, gray2, CV_BGR2GRAY);
    
    vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
#if 3
    auto sift = SIFT::create();
    BFMatcher matcher;
#else
    auto detector = FeatureDetector::create("SIFT");
    auto extractor = DescriptorExtractor::create("SIFT");
    auto matcher = DescriptorMatcher::create("BruteForce");
#endif

#if 3
    sift->detect(gray1, keypoints1);
    sift->detect(gray2, keypoints2);
    sift->compute(gray1, keypoints1, descriptors1);
    sift->compute(gray2, keypoints2, descriptors2);
#else
    if(detector.empty()){
        cout << "empty" << endl;
        return 0;
    }

    detector->detect(gray, keypoints);

    return 0;

    extractor->compute(gray, keypoints, descriptors);
#endif
    vector<vector<DMatch>> matches;
    matcher.knnMatch(descriptors1, descriptors2, matches, 2);

    struct IndexPair{
        int tIndex;
        int qIndex;
    };

    vector<IndexPair> indexs;

    for(auto iter : matches){
        if(iter.size() == 2 && iter[0].distance < iter[1].distance * 0.75){
            indexs.push_back({iter[0].trainIdx, iter[0].queryIdx});
        }
    }

    vector<KeyPoint> kp1, kp2;
    vector<Point2f> pf1, pf2;

    for(auto iter : indexs){
        kp1.push_back(keypoints1[iter.qIndex]);
        kp2.push_back(keypoints2[iter.tIndex]);
    }

    KeyPoint::convert(kp1, pf1);
    KeyPoint::convert(kp2, pf2);

    Mat H = findHomography(pf1, pf2, RANSAC, 4);

    warpPerspective(img1, dst1, H, img1.size() * 2);
    Mat tmp = dst1(Rect(0, 0, img2.cols, img2.rows));
    img2.copyTo(tmp);
    //dst1.adjustROI(0, img2.size().height - 1, 0, img2.size().width - 1);
    //img2.copyTo(dst1);
    //dst1.adjustROI(0, dst1.size().height - 1, 0, dst1.size().width - 1);
    //Mat tmp = dst1(Range(0, img2.size().height - 1), Range(0, img2.size().width));
    //imshow("tmp", tmp);
    //imshow("gray1", gray1);
    imshow("dst1", dst1);
    //imshow("gray2", gray2);
    //imshow("img2", img2);
    //imshow("desc1", descriptors1);
    //imshow("desc2", descriptors2);
    waitKey(0);

    return 0;
}
