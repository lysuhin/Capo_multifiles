#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QImage>
#include <QQuickImageProvider>

#include "opencv2/opencv.hpp"

using namespace cv;

inline QImage  cvMatToQImage( const cv::Mat &inMat )
{
   switch ( inMat.type() )
   {
      // 8-bit, 4 channel
      case CV_8UC4:
      {
         QImage image( inMat.data,
                       inMat.cols, inMat.rows,
                       static_cast<int>(inMat.step),
                       QImage::Format_ARGB32 );

         return image;
      }

      // 8-bit, 3 channel
      case CV_8UC3:
      {
         QImage image( inMat.data,
                       inMat.cols, inMat.rows,
                       static_cast<int>(inMat.step),
                       QImage::Format_RGB888 );

         return image.rgbSwapped();
      }

      // 8-bit, 1 channel
      case CV_8UC1:
      {
         static QVector<QRgb>  sColorTable( 256 );

         // only create our color table the first time
         if ( sColorTable.isEmpty() )
         {
            for ( int i = 0; i < 256; ++i )
            {
               sColorTable[i] = qRgb( i, i, i );
            }
         }

         QImage image( inMat.data,
                       inMat.cols, inMat.rows,
                       static_cast<int>(inMat.step),
                       QImage::Format_Indexed8 );

         image.setColorTable( sColorTable );

         return image;
      }

      default:
//         qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
         break;
   }

   return QImage();
}


class ColorImageProvider : public QQuickImageProvider
{
public:
    ColorImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize)
    {

        Mat img = Mat::zeros(200, 300, CV_8UC3);
        putText(img, "OpenCV", Point2f(20,120), FONT_HERSHEY_DUPLEX, 2, Scalar(0,255,255,255));

        QImage image = cvMatToQImage(img);


        return image;
    }
};

int main(int argc, char *argv[])
{


    QGuiApplication app(argc, argv);



    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("colors"), new ColorImageProvider);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
