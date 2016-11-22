#ifndef PERIVASCULAR_H
#define PERIVASCULAR_H



class Perivascular
{
    private:

        float DECAY_COEFFICIENT = 1.1;
        size_t BLOCK_SIZE = 64;

    public:

        double  coefficient;
        double  mean_width;
        size_t  block_size;
        bool    verbose;
        vector  <double> widths;

        Mat image;

        Perivascular(){
            coefficient = DECAY_COEFFICIENT;
            block_size = BLOCK_SIZE;
            verbose = 1;
        }

        Perivascular(float c, size_t b, size_t v = 1){
            coefficient = c;
            block_size = b;
            verbose = v;
        }

        ~Perivascular() {}

        void applyFiltering(Mat source, size_t clipLimit=4, size_t lut_param=8, size_t ksize=7){

            Mat     lab_image, L, clahed,
                    binned, smoothed;

            cvtColor(source, lab_image, CV_BGR2Lab);

            extractChannel(lab_image, L, 0);
            normalize(L, L, 0, 255, NORM_MINMAX);
            //imshow ("L", L);
            lab_image.release();

            equalizeHist(L, L);

            Ptr<CLAHE> clahe = createCLAHE();
            clahe->setClipLimit(4);
            clahe->apply(L, clahed);
            //imshow("Clahe", clahed);
            L.release();

            Mat lookuptable(1, 256, CV_8U);
            uchar* p = lookuptable.data;
            size_t BINS = 256 / lut_param;
            for (size_t i = 0; i < 256; i++)
                p[i] =  (i / BINS) * BINS;

            LUT(clahed, lookuptable, binned);
            //imshow("LUT", binned);
            clahed.release();

            //bilateralFilter(binned, smoothed, 8, 90, 90);
            medianBlur(binned, smoothed, ksize);
            //imshow("Smoothed", smoothed);
            binned.release();

            image = smoothed;
        }

        void calculate(Mat img, vector <Point2d> points, bool needs_drawing = false){

            applyFiltering(img);

            size_t  max_int_y, found = 0,
                    check_distance = 100;

            unsigned char   temp_intensity,
                            max_int;

            for (size_t i_point = 0; i_point < points.size(); i_point++){                            /// ordering loop by i_point
                Point2d point = points.at(i_point);
                if (point.y - check_distance > 0)
                {
                    mean_width = 0.;
                    for (size_t j = point.x; j < point.x + block_size; j++)                         /// horizontal loop by j
                    {
                        max_int = 0;
                        max_int_y = point.y;

                        for (size_t i = point.y; i > point.y - check_distance; i--)                 /// vertical loop #1 by i
                        {
                            temp_intensity = image.at<unsigned char>(i, j);
                            if (temp_intensity >= max_int){
                                max_int = temp_intensity;
                                max_int_y = i;
                            }
                        }

                        for (size_t i = max_int_y; i > point.y - check_distance; i--)                 /// vertical loop #2 by i
                        {
                            temp_intensity = image.at<unsigned char>(i, j);
                            if (temp_intensity <= (double) max_int / coefficient)
                            {                                                                         /// update mean for block
                                mean_width += (point.y - i) / (double) block_size;
                                found = 1;
                                //circle(image, Point(j, i), 1, 1);
                                break;
                            }
                        }
                        if (found == 0)
                        {
                            mean_width += ((double) check_distance) / block_size;
                            //circle(image, Point(j, point.y - check_distance), 1, 1);
                        }
                        found = 0;
                    }
                    widths.push_back(mean_width);                                                       /// update vector of widths
                    if (needs_drawing)
                    {
                        line(image, Point2f(point.x + block_size / 2, point.y), Point2f(point.x + block_size / 2, point.y - mean_width), 1, 2);
                        line(image, Point2f(point.x, point.y), Point2f(point.x + block_size, point.y), 1, 2);
                        line(image, Point2f(point.x, point.y - mean_width), Point2f(point.x + block_size, point.y - mean_width), 1, 2);
                    }
                }
            }
            mean_width = 0;
            for (size_t i = 0; i < widths.size(); i++)
            {
                mean_width += widths.at(i);
                if (verbose) cout << "Width #" << i << " = " << widths.at(i) << endl;
            }
            mean_width /= widths.size();
            if (verbose) cout << "Mean width = " << mean_width << endl;
        }

        void drawPerivascular(Mat image, vector <Point2d> &points, vector <float> &widths){
            for (size_t i = 0; i < points.size(); i++)
            {
                Point2d point = points.at(i);
                float mean_width = widths.at(i);
                line(image, Point2f(point.x + block_size / 2, point.y), Point2f(point.x + block_size / 2, point.y - mean_width), 1, 2);
                line(image, Point2f(point.x, point.y), Point2f(point.x + block_size, point.y), 1, 2);
                line(image, Point2f(point.x, point.y - mean_width), Point2f(point.x + block_size, point.y - mean_width), 1, 2);
            }
        }
};

#endif // PERIVASCULAR_H
