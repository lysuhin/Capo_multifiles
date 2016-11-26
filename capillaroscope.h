#ifndef CAPILLAROSCOPE_H
#define CAPILLAROSCOPE_H

#include "detection.h"
#include "perivascular.h"

/// facade? creator?

class Capillaroscope
{
    private:

        struct Results{

            double  width_of_perivascular_mean,
                    width_of_perivascular_std,
                    density_of_capillars;

            string  results_printable;

            void collectResults();
        };

    public:

        Detection*      detector;
        Perivascular*   perivascular;
        Results*        results;

        Mat     image_source,
                image_detected,
                image_perivascular;

        size_t  number_of_detected_points;

        bool    verbose,
                needs_calculation_perivascular;

        Capillaroscope(bool verbose = true)
        {
            this->verbose = verbose;
            this->detector = new Detection(verbose);
            //this->perivascular = new Perivascular(verbose);
            this->results = new Results;

            detector->train();
        }

        void detect(Mat img,
                    bool needs_upper = false,
                    bool needs_drawing_squares = true,
                    bool needs_points_filtering = false,
                    bool needs_calculation_perivascular = false,
                    bool needs_drawing_widths = false)
        {
            this->image_source = img;

            if (needs_drawing_squares)                                  /// (copy image) or (clone header)
                this->image_source.copyTo(this->image_detected);        /// depending on need to edit image
            else
                this->image_detected = image_source;

            detector->detect(image_detected, needs_points_filtering, needs_drawing_squares, needs_upper);
            this->number_of_detected_points = detector->points_result.size();

            if (needs_calculation_perivascular)
            {
                this->perivascular = new Perivascular(verbose);

                if (needs_drawing_widths)
                    this->image_source.copyTo(this->image_perivascular);
                else
                    this->image_perivascular = image_source;

                perivascular->calculate(image_perivascular, detector->points_result, needs_drawing_widths);
                this->results->width_of_perivascular_mean = perivascular->mean_width;
                this->results->width_of_perivascular_std = perivascular->std_width;
            }
        }

        ~Capillaroscope() {}

};

#endif // CAPILLAROSCOPE_H
