#include "FlyCapture2.h"
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define _FRAMERATE      FRAMERATE_15
#define _VIDEOMODE      VIDEOMODE_1280x960Y8

using namespace cv;
using namespace FlyCapture2;
using namespace std;

boost::mutex eoMutex, irMutex;
int width = 1280;
int height = 960;
int eoIndex = 0;
int irIndex = 0;
bool finishedCapture= false;
queue<Image> eoImages;
queue<Mat> irImages;
PGMOption option;

void
  PrintCameraInfo(CameraInfo* pCamInfo)
{
  printf(
    "\n*** CAMERA INFORMATION ***\n"
    "Serial number - %u\n"
    "Camera model - %s\n"
    "Camera vendor - %s\n"
    "Sensor - %s\n"
    "Resolution - %s\n"
    "Firmware version - %s\n"
    "Firmware build time - %s\n\n",
    pCamInfo->serialNumber,
    pCamInfo->modelName,
    pCamInfo->vendorName,
    pCamInfo->sensorInfo,
    pCamInfo->sensorResolution,
    pCamInfo->firmwareVersion,
    pCamInfo->firmwareBuildTime);
}

void
  PrintError(Error error)
{
  error.PrintErrorTrace();
}

void
  saveFrameToFile(Image *frame, string filename, int width, int height)
{
  FILE* imagefile = fopen(filename.c_str(), "wb");
  if (imagefile == NULL) {
    cout << "Unable to open file! - " << filename << "\n";
  }
  uint64_t numBytes;
  numBytes = (960 * 1280) * 1;
  fprintf(imagefile, "P5\n%u %u\n255\n", width, height);

  try {
    if (frame->GetRows() == height && frame->GetCols() == width)
      fwrite(frame->GetData(), 1, numBytes, imagefile);
    else {
      cout << "empty image...\n";
    }
  }
  catch (int e) {
    cout << "error writing file " << filename << "- " << e << ".\n";
    fflush(imagefile);
    fclose(imagefile);
  }
  fflush(imagefile);
  fclose(imagefile);
}

int
  captureFrame(Camera& cam, Image& rawImage, Error& error)
{
  error = cam.RetrieveBuffer(&rawImage);
  Image tmpImage;

  if (error != PGRERROR_OK) {
    printf("error grabbing image\n");
    eoMutex.lock();
    tmpImage.DeepCopy(&eoImages.back()); // make it same as most recent image
    eoMutex.unlock();
  }
  else {
    tmpImage.DeepCopy(&rawImage);
  }

  eoMutex.lock();
  eoImages.push(tmpImage);
  eoMutex.unlock();

  return 0;
}

void
  saveImages(const string& rootnameIR, const string& rootnameEO, const int numImages)
{
  Mat irFrame;
  Image frame;

  int size;
  int sizeIR;

  while (eoIndex < numImages || irIndex < numImages) {

    size = 0;
    sizeIR = 0;

    eoMutex.lock();
    size = (int)eoImages.size();
    eoMutex.unlock();

    irMutex.lock();
    sizeIR = (int)irImages.size();
    irMutex.unlock();

    bool grabError;

    if (sizeIR > 0) {
      string out_name(rootnameIR.c_str());
      out_name.append(boost::lexical_cast<string>(irIndex - 7));
      out_name.append(".png");

      irMutex.lock();
      irFrame = irImages.front();
      irImages.pop();
      irMutex.unlock();

      if (irIndex >= 7) {
        IplImage* tmp = new IplImage(irFrame);
        cvSaveImage(out_name.c_str(), tmp);
      }
      irIndex++;
    }

    if (size > 0) {

      string filename(rootnameEO.c_str());
      filename.append(boost::lexical_cast<string>(eoIndex));
      filename.append(".pgm");

      eoMutex.lock();
      frame = eoImages.front();
      eoMutex.unlock();

      eoMutex.lock();
      eoImages.pop();
      eoMutex.unlock();

      saveFrameToFile(&frame, filename, width, height);
  
      eoIndex++;
    }

  }

}

int
  captureFrames(PGRGuid guid, const int numImages)
{
  VideoCapture cap;
  cap.open(200); // make sure it's getting the right cam

  if (!cap.isOpened()) {
    cout << "***Could not initialize capturing...***\n";
    return -1;
  }

  Mat irFrame;
  Rect ROI(70, 53, 584, 474);

  try {

    Error error;
    Camera cam;

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
      PrintError(error);
      return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
      PrintError(error);
      return -1;
    }

    // Set the VideoMode and FrameRate
    error = cam.SetVideoModeAndFrameRate(_VIDEOMODE, _FRAMERATE);

    if (error != PGRERROR_OK)
    {
      PrintError(error);
      return -1;
    }

    PrintCameraInfo(&camInfo);

    // Start capturing images
    printf("Started capture... \n");
    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
      PrintError(error);
      return -1;
    }

    // Set the shutter property of the camera
    Property prop;
    prop.type = SHUTTER;
    error = cam.GetProperty(&prop);
    if (error != PGRERROR_OK)
    {
      PrintError(error);
      return -1;
    }

    prop.autoManualMode = false;
    prop.absControl = true;

    Image rawImage;
    for (int i = 0; i < numImages; i++) {
      boost::thread captureFrameThread(captureFrame, boost::ref(cam), boost::ref(rawImage), boost::ref(error));

      try {
        cap >> irFrame;
        irFrame = irFrame(ROI);

        irMutex.lock();
        irImages.push(irFrame);
        irMutex.unlock();
      }
      catch (int e) {
        irMutex.unlock();
        cerr << "error " << e << " in infrared capture\n";
      }

      captureFrameThread.join();

    }

    finishedCapture = true;
    printf("Stopped capture... \n");
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
      PrintError(error);
      return -1;
    }

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
      PrintError(error);
      return -1;
    }


  }
  catch (int e) {
    eoMutex.unlock();
    irMutex.unlock();
    cerr << "error code: " << e << endl;
  }
  return 0;
}

bool
  isFolderWritable()
{
  FILE* tempFile = fopen("test.txt", "w+");
  if (tempFile == NULL)
  {
    fclose(tempFile);
    remove("test.txt");
    return false;
  }
  fclose(tempFile);
  remove("test.txt");
  return true;
}

int main(int argc, char *argv[]) {

  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " num_images timestamp" << endl;
    return 1;
  }

  option.binaryFile = true;
  const int numImages = atoi(argv[1]);
  const string timestamp = argv[2];

  if (!isFolderWritable()) {
    cerr << "Cannot write to current dir. Exiting..." << std::endl;
    return 1;
  }

  BusManager busMgr;
  unsigned int numCameras;
  Error error = busMgr.GetNumOfCameras(&numCameras);
  if (error != PGRERROR_OK)
  {
    PrintError(error);
    return -1;
  }

  if (numCameras < 1)
  {
    printf("No camera detected.\n");
    return -1;
  }
  else
  {
    printf("Number of cameras detected: %u\n", numCameras);
  }

  PGRGuid guid;
  error = busMgr.GetCameraFromIndex(0, &guid);
  if (error != PGRERROR_OK)
  {
    PrintError(error);
    return -1;
  }

  // get the current timestamp and create a directory for it
  string rootnameEO = timestamp + "/eo/";
  string rootnameIR = timestamp + "/ir/";
  mkdir(timestamp.c_str(), 0777);
  mkdir(rootnameEO.c_str(), 0777);
  mkdir(rootnameIR.c_str(), 0777);

  boost::thread saveThread(saveImages, rootnameIR, rootnameEO, numImages);
  boost::thread captureThread(captureFrames, guid, numImages);
  saveThread.join();

  return 0;
}
