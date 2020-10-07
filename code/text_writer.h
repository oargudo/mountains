/*
 * Oscar Argudo: adapted from Andrew Kirmse kml_writer.h
 */


#ifndef _TEXT_WRITER_H_
#define _TEXT_WRITER_H_

// Utilities for building up the divide tree as text

#include "primitives.h"
#include "coordinate_system.h"

#include <string>

class TextWriter {
public:

  // Coordinate system is used to convert offsets to lat/lngs
  explicit TextWriter(const CoordinateSystem &coords);

  void startFolder(const std::string &name, unsigned int numElements);
  void endFolder();
  
  void addPeak(const Peak &peak, const std::string &name);
  void addRunoff(const Runoff &runoff, const std::string &name);
  void addPromSaddle(const Saddle &saddle, const std::string &name);
  void addBasinSaddle(const Saddle &saddle, const std::string &name);

  void addGraphEdge(const Peak &peak1, const Peak &peak2, const Saddle &saddle, const std::string& peakName, const std::string& parentName, const std::string& saddleName);
  void addRunoffEdge(const Peak &peak, const Runoff &runoff, const std::string& peakName, const std::string& runoffName);
  void addPeakEdge(const Peak &peak1, const Peak &peak2, const std::string& peakName1, const std::string& peakName2);

  // End document and return KML
  std::string finish();
  
private:
  const CoordinateSystem &mCoords;
  std::string mTxt;

  void addSaddle(const Saddle &saddle, const char *styleUrl, const std::string &name);
};

#endif  // ifndef _KML_WRITER_H_
