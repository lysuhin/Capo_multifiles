#ifndef DETECTION_H_INCLUDED
#define DETECTION_H_INCLUDED

#include "opencv2/ml/ml.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace cv::ml;
using namespace std;

class Detection{

    private:

        string  TRAINING_EXAMPLES_POS_PATH = "examples/train_positive/",
                TRAINING_EXAMPLES_NEG_PATH = "examples/train_negative/",
                TESTING_EXAMPLES_POS_PATH = "examples/test_positive/",
                TESTING_EXAMPLES_NEG_PATH = "examples/test_negative/",
                IMAGE_FORMAT = "_.jpg",
                MODEL_LOCATION = "examples/svm_model.xml";

        size_t  NUM_OF_POS_EXAMPLES = 465,
                NUM_OF_NEG_EXAMPLES = 422,
                NUM_OF_POS_TEST_EXAMPLES = 100,
                NUM_OF_NEG_TEST_EXAMPLES = 100;

    public:

       size_t   windows_n_rows = 64,
                windows_n_cols = 64,
                StepSlideRows = 32,
                StepSlideCols = 32;

        bool    verbose,
                needs_filtration;

        Ptr<SVM>    svm;
        Mat     image;


        Detection(bool vrbs, bool nds_fltrtn){
            verbose = vrbs;
            needs_filtration = nds_fltrtn;
        }

        Detection(){
            verbose = true,
            needs_filtration = true;
        };

        Mat applyFiltering(Mat img){
            Mat img_median,
                img_clahe,
                green,
                green_f;

            vector <Mat> channels;

            split(img, channels);
            green = channels[1];
            equalizeHist(green, green);

            Ptr<CLAHE> clahe = createCLAHE();
            clahe->setClipLimit(4);
            clahe->apply(green, img_clahe);

            bilateralFilter(img_clahe, green_f, 8, 90, 90);
            img_median.release();
            img_clahe.release();
            green.release();
            return green_f;
        }

        Mat calcGradient(Mat src_gray){

            int scale = 1,
                delta = 0,
                ddepth = CV_16S;

            Mat grad_x, grad_y, grad;
            Mat abs_grad_x, abs_grad_y;

            Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_REFLECT_101 );
            convertScaleAbs( grad_x, abs_grad_x );

            Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_REFLECT_101 );
            convertScaleAbs( grad_y, abs_grad_y );

            addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
            grad_x.release(); grad_y.release(); abs_grad_x.release(); abs_grad_y.release();
            return grad;
        }

        void computeHOGs(
                    bool type_of_examples,
                    string PATH,
                    size_t NUM_OF_EXAMPLES,
                    vector <int>& labels,
                    vector <vector <float>>& features){

            vector <float> descriptors;
            vector <Point> locations;
            string example_name;

            for (size_t i = 0; i < NUM_OF_EXAMPLES + 1; i++){
                example_name = PATH + to_string(i) + IMAGE_FORMAT;
                Mat example_image = imread(example_name);
                if (!example_image.data) continue;
                resize(example_image, example_image, Size(64, 64));

                // PARAMETERS OF HOGDESCRIPTOR: WinSize, BlockSize, BlockStride, CellSize, nbins, aperture, WinSigma
                HOGDescriptor* hog = new HOGDescriptor(Size(64, 64), Size(16, 16), Size(8, 8), Size(16, 16), 9, 1, -1);
                hog->compute(example_image, descriptors, Size(8, 8), Size(0, 0), locations);
                features.push_back(descriptors);
                labels.push_back(type_of_examples);
                example_image.release();
                delete hog;
            }
        }

        void createFeatureMat(Mat& matrix, vector <vector <float>>& features){
            for (size_t i = 0; i < features.size(); i++)
                for (size_t j = 0; j < features.back().size(); j++)
                    matrix.at<float>(i, j) = features[i][j];
        }

        double calculateAccuracy(Ptr<SVM> svm, Mat testingDataMat, Mat testingLabelsMat, bool verbose){
            Mat query;
            size_t right_answers = 0;
            for (size_t i = 0; i < testingLabelsMat.rows; i++){
                Mat query = testingDataMat.row(i);
                int answer = svm->predict(query);
                if (answer == testingLabelsMat.at<int>(i))
                    right_answers++;
                if (verbose) cout << "Predicted: " << answer << " | True: " << testingLabelsMat.at<int>(i) << endl;
            }
            double accuracy = (double) right_answers / testingLabelsMat.rows;
            return accuracy;
        }

        void train(){
            vector <int>    train_labels,
                            test_labels;

            vector <vector<float>>  train_features,
                                    test_features;

            if (verbose) cout << "computing hogs for training...";

            computeHOGs(1, TRAINING_EXAMPLES_POS_PATH, NUM_OF_POS_EXAMPLES, train_labels, train_features);
            computeHOGs(0, TRAINING_EXAMPLES_NEG_PATH, NUM_OF_NEG_EXAMPLES, train_labels, train_features);
            computeHOGs(1, TESTING_EXAMPLES_POS_PATH, NUM_OF_POS_TEST_EXAMPLES, test_labels, test_features);
            computeHOGs(0, TESTING_EXAMPLES_NEG_PATH, NUM_OF_NEG_TEST_EXAMPLES, test_labels, test_features);

            if (verbose) cout << "computed." << endl << "training...";

            Mat trainingDataMat(train_features.size(), train_features.front().size(), CV_32F);
            createFeatureMat(trainingDataMat, train_features);
            Mat trainingLabelsMat = Mat(train_labels);

            Mat testingDataMat(test_features.size(), test_features.front().size(), CV_32F);
            createFeatureMat(testingDataMat, test_features);
            Mat testingLabelsMat = Mat(test_labels);

            #if 1
            svm = SVM::create();
            Ptr<TrainData> trdata = TrainData::create(trainingDataMat, ROW_SAMPLE, trainingLabelsMat);
            svm->trainAuto(trdata);
            svm->save(MODEL_LOCATION);
            #else
            svm = SVM::load<SVM>(MODEL_LOCATION);
            svm->train(trainingDataMat, ROW_SAMPLE, trainingLabelsMat);
            #endif // 0

            if (verbose){
                cout << "trained." << endl;
                cout << "type = \t\t\t\t" << svm->getType() << endl;
                cout << "kernel = \t\t\t" << svm->getKernelType() << endl;
                cout << "gamma = \t\t\t" << svm->getGamma() << endl;
                cout << "C = \t\t\t\t" << svm->getC() << endl;
                cout << "Train accuracy is \t\t" << calculateAccuracy(svm, trainingDataMat, trainingLabelsMat, 0) << endl;
                cout << "Test accuracy is \t\t" << calculateAccuracy(svm, testingDataMat, testingLabelsMat, 0) << endl;
            }

            trainingDataMat.release();
            trainingLabelsMat.release();
            testingDataMat.release();
            testingLabelsMat.release();
        }

        void detect(Mat LoadedImage, vector <Point2d> &points){

            vector <float>  descriptor;
            vector <Point>  location;

            size_t      found = false;

            if (needs_filtration)
                image = calcGradient(applyFiltering(LoadedImage));
            else
                LoadedImage.copyTo(image);

            for (size_t row = 0; row <= image.rows - windows_n_rows; row += StepSlideRows)
            {
                for (size_t col = 0; col <= image.cols - windows_n_cols; col += StepSlideCols)
                {
                    ///creating window for ROI
                    Rect windows(col, row, windows_n_rows, windows_n_cols);
                    Mat Roi = image(windows);

                    ///Everything needed for HOG
                    resize(Roi, Roi, Size(64, 64));
                    HOGDescriptor* hog = new HOGDescriptor(Size(64, 64), Size(16, 16), Size(8, 8), Size(16, 16), 9, 1, -1);
                    hog->compute(Roi, descriptor, Size(8, 8), Size(0, 0), location);
                    Mat query = Mat(descriptor);
                    transpose(query, query);
                    found = svm->predict(query);
                    query.release();
                    delete hog;

                    if (found){
                        points.push_back(Point2d(col, row));
                    }
                }
            }
        }
        ~Detection();
};


#endif // DETECTION_H_INCLUDED
