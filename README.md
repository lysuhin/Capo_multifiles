#Capo_multifiles

capillaroscopy .cbp project

##Structure of project:

* **main.cpp** : calls Capillaroscope::detect() from *capillaroscope.h*.

* **capillaroscope.h** : contains facade class "Capillaroscope" that provides whole detection and calculation given an image and parameter *bool verbose* if needed to print out the inner logs. Numeric results are stored in structure *result* and visual results are stored as *Mat image_detected* and *Mat image_perivascular* as class' attributes.
Necessary parameters for Detection::detect(...) and Perivascular::calculate(...) are passed to Capillaroscope::detect(...):
   
   *void detect(Mat img, bool needs_upper = false, bool needs_drawing_squares = true, bool needs_points_filtering = false,
               bool needs_calculation_perivascular = false, bool needs_drawing_widths = false)*

* **detection.h** : contains class "Detection" that, having received image source and/or parameters of detection, writes coordinates of found capillar positions into vector. For outer use only detect(...) method is required.
Method Detection::detect(...) has parameters as follows:

  * *Mat image* : source image to detect capillary system on;
  * *bool needs_points_filtration* : for filtering points procedure that detects the areas of the most probable capillars positions on image and filters out those that are not in these positions;
  * *bool needs_drawing* : for drawing squares on found positions;
  * *bool needs_upper* : for choosing only those capillars that lay in upper row of capillary net.
  
  Resulting coordinates of capillars are stored in *vector \<Point2d\> points_result*, and the image with filters applied and drawings (if chosen) is stored in *image*.

  **NB** : due to [unfixed issue] (https://github.com/opencv/opencv/issues/5054) in opencv/ml module every run of app starts with training SVM parameters (with hyperparameters fixed). Training examples for 64x64 objects are stored in */examples* as well as *svm_model.xml* with pre-tuned hyperparameters.

* **perivascular.h** : contains class "Perivascular" that, having received vector of Point2d-coordinates of first-row-capillars, calculates and stores mean width of each capillar's perivascular space in *vector \<double\> widths* and the overall mean value in *mean_width*. 
Method Perivascular::calculate(...) has parameters as follows:
  * *Mat image* : source image to calculate perivascular space on;
  * *vector \<Points2d\> points* : coordinates of found first-row-capillars;
  * *bool needs_drawing* : for drawing mean widths.
