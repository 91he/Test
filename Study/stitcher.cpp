#include <iostream>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"

using namespace std;
using namespace cv;

void printUsage()
{
    cout <<
        "Rotation model images stitcher.\n\n"
        "stitching img1 img2 [...imgN]\n\n"
        "Flags:\n"
        "  --try_use_gpu (yes|no)\n"
        "      Try to use GPU. The default value is 'no'. All default values\n"
        "      are for CPU mode.\n"
        "  --output <result_img>\n"
        "      The default is 'result.jpg'.\n";
}

bool try_use_gpu = false;
vector<Mat> imgs;
string result_name = "result.jpg";

int parseCmdArgs(int argc, char** argv)
{
    if (argc == 1)
    {
        printUsage();
        return -1;
    }
    for (int i = 1; i < argc; ++i)
    {
        if (string(argv[i]) == "--help" || string(argv[i]) == "/?")
        {
            printUsage();
            return -1;
        }
        else if (string(argv[i]) == "--try_gpu")
        {
            if (string(argv[i + 1]) == "no")
                try_use_gpu = false;
            else if (string(argv[i + 1]) == "yes")
                try_use_gpu = true;
            else
            {
                cout << "Bad --try_use_gpu flag value\n";
                return -1;
            }
            i++;
        }
        else if (string(argv[i]) == "--output")
        {
            result_name = argv[i + 1];
            i++;
        }
        else
        {
            Mat img = imread(argv[i]);
            if (img.empty())
            {
                cout << "Can't read image '" << argv[i] << "'\n";
                return -1;
            }
            imgs.push_back(img);
        }
    }
    return 0;
}


int main(int argc, char* argv[])
{
    int retval = parseCmdArgs(argc, argv);
    if (retval) return -1;

    Mat pano;
    Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
    Stitcher::Status status = stitcher.stitch(imgs, pano);

    if (status != Stitcher::OK)
    {
        cout << "Can't stitch images, error code = " << status << endl;
        return -1;
    }

    imwrite(result_name, pano);
    return 0;
}
