// Library imports
#include <fstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/opencv.hpp>

// Namespace imports
using namespace std;
using namespace cv;
using namespace cv::dnn;

int main(int argc, char *argv[]) {
	int personCount = 0;

	if (argc < 2) { // Check if there are enough args
		printf("ERROR: Insufficient arguments!\nPlease specify an image.\n");
		return (1);
	}

	ifstream ifs( // Read in the class names from the file
		string("./inputData/object_detection_classes_coco.txt").c_str());
	string line;

	vector<string> classNames;

	while (getline(ifs, line)) {
		classNames.push_back(line);
	}

	float minConfidence = 0.3; // Minimum confidence to be detected

	auto imgNet = readNet( // Read the Tensors from the file
		"./inputData/frozen_inference_graph.pb",
		"./inputData/ssd_mobilenet_v1_coco.pbtxt", "TensorFlow");

	Mat image = imread(argv[1]); // Read the image specified by the user

	int imageHeight = image.rows;
	int imageWidth = image.cols;

	Mat blob = blobFromImage( // Create a blob from the input image
		image, 1.0, Size(300, 300), Scalar(127.5, 127.5, 127.5), true, false);

	imgNet.setInput(blob); // Set the input of the network to the blob

	Mat output = imgNet.forward();

	Mat results(output.size[2], output.size[3], CV_32F, output.ptr<float>());

	for (int i = 0; i < results.rows; ++i) {
		int classID = int(
			results.at<float>(i, 1)); // Get the class ID of the detected object
		float confidence = results.at<float>(
			i, 2); // Get the confidence of the detected object

		if (confidence > minConfidence) {
			int boxX = int(results.at<float>(i, 3) * imageWidth);
			int boxY = int(results.at<float>(i, 4) * imageHeight);
			int boxWidth = int(results.at<float>(i, 5) * image.cols - boxX);
			int boxHeight = int(results.at<float>(i, 6) * image.rows - boxY);
			rectangle( // Draw a rectangle around the detected entity
				image, Point(boxX, boxY),
				Point(boxX + boxWidth, boxY + boxHeight), Scalar(0, 255, 0),
				2);
			string className = classNames[classID - 1];
			if (className == "person") {
				personCount++;
			}
			putText( // Write the detected enity type on the image
				image, className + " " + to_string(int(confidence * 100)) + "%",
				Point(boxX, boxY - 5), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 2);
		}
	}
	// Show the image with the detected entities highlighted
	imshow("image", image);

	if (personCount > 0) { // If there are people in the image
		cout << "People Detected.\nIt is not safe to turn the light green!\n";
	} else {
		cout << "No People Detected.\nIt is safe to turn the light green!\n";
	}
	waitKey(0);
	int k = waitKey(10);
	if (k == 27) {
		destroyAllWindows();
	}
}