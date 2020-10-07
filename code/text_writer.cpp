/*
 * Oscar Argudo: adapted from Andrew Kirmse kml_writer.cpp
 */

#include "text_writer.h"

using std::string;

TextWriter::TextWriter(const CoordinateSystem &coords)
    : mCoords(coords) {
}

void TextWriter::startFolder(const std::string &name, unsigned int numElements) {
  mTxt += name + " " + std::to_string(numElements) + "\n";
}

void TextWriter::endFolder() {
}

void TextWriter::addPeak(const Peak &peak, const std::string &name) {
  LatLng peakpos = mCoords.getLatLng(peak.location);
  mTxt += name + " ";
  mTxt += std::to_string(peakpos.longitude());
  mTxt += " ";
  mTxt += std::to_string(peakpos.latitude());
  mTxt += " ";
  mTxt += std::to_string(peak.elevation);
  mTxt += " ";
  mTxt += std::to_string(peak.location.x());
  mTxt += " ";
  mTxt += std::to_string(peak.location.y());
  mTxt += "\n";
}

void TextWriter::addPromSaddle(const Saddle &saddle, const std::string &name) {
  addSaddle(saddle, "#saddle", name);
}

void TextWriter::addBasinSaddle(const Saddle &saddle, const std::string &name) {
  addSaddle(saddle, "#basinsaddle", name);
}

void TextWriter::addSaddle(const Saddle &saddle, const char *styleUrl, const string &name) {
  LatLng saddlepos = mCoords.getLatLng(saddle.location);
  mTxt += name + " ";
  mTxt += std::to_string(saddlepos.longitude());
  mTxt += " ";
  mTxt += std::to_string(saddlepos.latitude());
  mTxt += " ";
  mTxt += std::to_string(saddle.elevation);
  mTxt += " ";
  mTxt += std::to_string(saddle.location.x());
  mTxt += " ";
  mTxt += std::to_string(saddle.location.y());
  mTxt += "\n";
}

void TextWriter::addRunoff(const Runoff &runoff, const std::string &name) {
  LatLng pos = mCoords.getLatLng(runoff.location);
  mTxt += name + " ";
  mTxt += std::to_string(pos.longitude());
  mTxt += " ";
  mTxt += std::to_string(pos.latitude());
  mTxt += " ";
  mTxt += std::to_string(runoff.elevation);
  mTxt += " ";
  mTxt += std::to_string(runoff.location.x());
  mTxt += " ";
  mTxt += std::to_string(runoff.location.y());
  mTxt += "\n";
}

void TextWriter::addGraphEdge(const Peak &peak1, const Peak &peak2, const Saddle &saddle, 
                const std::string& peakName, const std::string& parentName, const std::string& saddleName) {
  LatLng peak1pos = mCoords.getLatLng(peak1.location);
  LatLng saddlepos = mCoords.getLatLng(saddle.location);
  LatLng peak2pos = mCoords.getLatLng(peak2.location);
  
  mTxt += peakName + " " + parentName + " " + saddleName + "\n";
}

void TextWriter::addRunoffEdge(const Peak &peak, const Runoff &runoff, const std::string& peakName, const std::string& runoffName) {
  LatLng peakpos = mCoords.getLatLng(peak.location);
  LatLng runoffpos = mCoords.getLatLng(runoff.location);
  
  mTxt += peakName + " " + runoffName + "\n";
}

void TextWriter::addPeakEdge(const Peak &peak1, const Peak &peak2, const std::string& peakName1, const std::string& peakName2) {
  mTxt += peakName1 + " " + peakName2 + "\n";
}

string TextWriter::finish() {
  return mTxt;
}

