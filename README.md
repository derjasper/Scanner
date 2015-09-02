# Scanner
Extract a document from an image using opencv.

The scanner class provides some functions to extract a document (a piece of paper, Whiteboard, photo, ...) from an image.
It does edge detection on the input image and transforms it to a 2-dimensional document. It also does some adjustments,
liek brightness and contrast.

The main.cpp is a simple demo/test application. To use the scanner class, copy it in your project. The functions should
be self-explaining.

The class is aimed to be used in an Ubuntu SDK app with live preview of detected edges using the camera.

Feel free to test and contribute, this code is far from perfect.
