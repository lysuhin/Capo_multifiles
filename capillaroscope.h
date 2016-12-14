#ifndef CAPILLAROSCOPE_H
#define CAPILLAROSCOPE_H

#include "detection.h"
#include "perivascular.h"

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

        Mat     image_source,                       /// this Mat CAN NOT BE edited and is kept as CONST
                image_detected,                     /// this Mat CAN BE edited (painted with squares and so)
                image_perivascular;                 /// thus Mat CAN BE editer (...)

        size_t  number_of_detected_points;

        bool    verbose,
                needs_calculation_perivascular;

        Capillaroscope(bool verbose = true)
        {
            this->verbose = verbose;
            this->detector = new Detection(verbose);
            this->results = new Results;

            detector->train();
        }

        void detect(const Mat& img,
                    bool needs_upper = false,
                    bool needs_drawing_squares = true,
                    bool needs_points_filtering = false,
                    bool needs_calculation_perivascular = false,
                    bool needs_drawing_widths = false)
        {
            this->image_source = img;

            if (needs_drawing_squares || needs_drawing_widths)
                this->image_detected = (this->image_source).clone();    /// (copy image) or (clone header)
            else                                                        /// depending on need to edit image
                this->image_detected = this->image_source;

            detector->detect(image_detected, needs_points_filtering, needs_drawing_squares, needs_upper);
            this->number_of_detected_points = detector->points_result.size();
            if (needs_calculation_perivascular)
            {

                this->perivascular = new Perivascular(verbose);

                if (needs_drawing_widths)
                    this->image_perivascular = (this->image_source).clone();
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
