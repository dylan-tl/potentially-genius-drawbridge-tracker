# Drawbridge Tracker

The Drawbridge Tracker is a project for DigiKey's Potentially Genius series that leverages the Arduino Uno Q to detect if the Pulaski Bridge (a commute-ruining drawbridge) is up or down, alerting us to potential delays.

An Arduino Uno Q runs a local vision model specifically trained on images of this bridge to determine if the bridge is up or down. Then, a scale model controlled by another Arduino Uno Q reflects the real state of the bridge for all the office to see.

This repository has code for both of the Arduinos, the model running on the camera side ARduino Uno Q (plus a link to the model on Edge Impulse), and the 3D files for both the camera and the scale bridge.
