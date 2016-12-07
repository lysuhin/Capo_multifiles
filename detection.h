#ifndef DETECTION_H_INCLUDED
#define DETECTION_H_INCLUDED

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

        struct Point2d_counts{

            double col;
            double row;
            size_t size;

            Point2d_counts(){};

            Point2d_counts(double a, double b)
            {
                col = a;
                row = b;
                size = 1;
            };

            void convertToPoint2d(Point2d& point){
                point.x = col;
                point.y = row;
            }
        };

    public:

        size_t  windows_n_rows = 64,
                windows_n_cols = 64,
                STEP_SLIDE_ROWS_ROUGH = 32,
                STEP_SLIDE_COLS_ROUGH = 32,
                STEP_SLIDE_ROWS_PRECISE = 2,
                STEP_SLIDE_COLS_PRECISE = 2,

                DETECTION_THRESHOLD = 3,
                DETECTION_EPSILON = 30,

                CAPILLARS_ROW_WIDTH = 32;

        bool    verbose,
                needs_filtration,
                needs_upper;

        Ptr<SVM>    svm;

        Mat     image;

        vector <Point2d> points_result;

        Detection(bool verbose = true, bool needs_filtration = true)
        {
            this->verbose = verbose;
            this->needs_filtration = needs_filtration;
        }

        Mat applyFiltering(Mat img){

            Mat img_median,
                img_clahe,
                green,
                green_f;

            extractChannel(img, green, 1);
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

        void computeHOGs(bool type_of_examples,
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
            for (int i = 0; i < testingLabelsMat.rows; i++){
                Mat query = testingDataMat.row(i);
                int answer = svm->predict(query);
                if (answer == testingLabelsMat.at<int>(i))
                    right_answers++;
                if (verbose) cout << "Predicted: " << answer << " | True: " << testingLabelsMat.at<int>(i) << endl;
            }
            double accuracy = (double) right_answers / testingLabelsMat.rows;
            return accuracy;
        }

        void filterPoints(vector <Point2d_counts>& points_in, vector <Point2d>& points_out, size_t threshold){

            vector <Point2d> points_temp;

            for (size_t i = 0; i < points_in.size(); i++)
                if (points_in.at(i).size >= threshold)
                    points_temp.push_back(Point2d(points_in.at(i).col, points_in.at(i).row));

            points_out = points_temp;
        }

        void getFirstPoint(vector <Point2d>& points, Point2d& first){
            double minY = points.back().y;
            for (size_t i = 0; i < points.size(); i++){
                if (points.at(i).y < minY){
                    first.x = points.at(i).x;
                    first.y = points.at(i).y;
                    minY = first.y;
                }
            }
            cout << "First point: (" << first.x << ", " << first.y << ")" << endl;
        }

        void getFirstRow(vector <Point2d>& points_in, vector <Point2d>& points_out){
            Point2d upper_point;
            getFirstPoint(points_in, upper_point);
            double threshold = upper_point.y;
            for (size_t i = 0; i < points_in.size(); i++){
                if (points_in.at(i).y <= threshold + CAPILLARS_ROW_WIDTH){
                    points_out.push_back(points_in.at(i));
                }
            }
            cout << "Points in first row:" << points_out.size() << endl;
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

            #if 0
            FileStorage file;
            file.open("trainingDataMat.yml", cv::FileStorage::WRITE);
            file << "trainingDataMat" << trainingDataMat;
            file.open("trainingLabelsMat.yml", cv::FileStorage::WRITE);
            file << "trainingLabelsMat" << trainingLabelsMat;
            file.open("testingDataMat.yml", cv::FileStorage::WRITE);
            file << "testingDataMat" << testingDataMat;
            file.open("testingLabelsMat.yml", cv::FileStorage::WRITE);
            file << "testingLabelsMat" << testingLabelsMat;
            #endif // 0

            #if 0
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

        void detect(Mat img,
                    bool needs_points_filtering = false,
                    bool needs_drawing = false,
                    bool needs_upper = false){

            img.copyTo(image);
            vector <float>  descriptor;
            vector <Point>  location;
            vector <Point2d_counts> points_with_counts;

            points_with_counts.clear();
            size_t  found = false,
                    step_slide_cols = STEP_SLIDE_COLS_ROUGH,
                    step_slide_rows = STEP_SLIDE_ROWS_ROUGH;

            if (needs_points_filtering){
                step_slide_cols = STEP_SLIDE_COLS_PRECISE;
                step_slide_rows = STEP_SLIDE_ROWS_PRECISE;
            }


            if (needs_filtration)
                image = calcGradient(applyFiltering(image));

            for (size_t row = 0; row <= image.rows - windows_n_rows; row += step_slide_rows)
            {
                for (size_t col = 0; col <= image.cols - windows_n_cols; col += step_slide_cols)
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

                    if (found)
                    {
                        if (needs_points_filtering){
                            if (!points_with_counts.empty())
                            {
                                bool added = false;
                                for(size_t i = 0; i < points_with_counts.size(); i++)
                                {
                                    double distance = sqrt(pow(double(col)-points_with_counts[i].col,2) + pow(double(row)-points_with_counts[i].row,2));
                                    if (distance < DETECTION_EPSILON)
                                    {
                                        points_with_counts[i].col = (points_with_counts[i].size * points_with_counts[i].col + col) / (points_with_counts[i].size + 1);
                                        points_with_counts[i].row = (points_with_counts[i].size * points_with_counts[i].row + row) / (points_with_counts[i].size + 1);
                                        points_with_counts[i].size++;
                                        added = true;
                                        break;
                                    }
                                }
                                if (!added)
                                {
                                    points_with_counts.push_back(Point2d_counts(double(col), double(row)));
                                }
                            }
                            else
                            {
                                points_with_counts.push_back(Point2d_counts(double(col), double(row)));
                            }
                        }
                        else
                        {
                            points_with_counts.push_back(Point2d_counts(col, row));
                        }
                    }
                }
            }
            cvtColor(image, image, CV_GRAY2BGR);
            if (verbose) cout << "number of detected capillars : " << points_with_counts.size() << endl;
            if (needs_points_filtering)
            {
                filterPoints(points_with_counts, points_result, DETECTION_THRESHOLD);
                if (verbose) cout << "number of points after filtering : " << points_result.size() << endl;
            }
            else
            {
                filterPoints(points_with_counts, points_result, 1);
            }
            if (needs_drawing) drawDetected(image, points_result, 1);
            if (needs_upper)
            {
                vector <Point2d> points_first_row;
                getFirstRow(points_result, points_first_row);
                if (needs_drawing) drawDetected(image, points_first_row, 2);
                points_result.resize(points_first_row.size());
                copy(points_first_row.begin(), points_first_row.end(), points_result.begin());
                if (verbose) cout << "number of points in first row : " << points_first_row.size() << endl;
            }
        }

        void drawDetected(Mat& image, vector <Point2d>& points, size_t color){
            Scalar c;
            switch (color)
            {
                case 0 : c = Scalar(255, 0, 0); break;
                case 1 : c = Scalar(0, 255, 0); break;
                case 2 : c = Scalar(0, 0, 255); break;
            }
            for (size_t i = 0; i < points.size(); i++)
            {
                Point2d point1 = points.at(i);
                Point2d point2 = Point2d(point1.x + 64, point1.y + 64);
                rectangle(image, point1, point2, c, 1, 8, 0);
            }
        }

        ~Detection();
};


#endif // DETECTION_H_INCLUDED
