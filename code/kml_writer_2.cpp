/*
 * MIT License
 * 
 * Copyright (c) 2020 Oscar Argudo, based on Andrew Kirmse's KMLWriter
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "kml_writer_2.h"

using std::string;

KMLWriter2::KMLWriter2(const CoordinateSystem &coords, const std::vector<KMLPeakStyle>& peakStyles) : mCoords(coords) {
  mKml = "<kml xmlns=\"http://www.opengis.net/kml/2.2\"><Document>\n";

  mKml += "<Style id=\"peak\"><IconStyle><color>ff00ccff</color><scale>1.2</scale><Icon><href>http://maps.google.com/mapfiles/kml/shapes/triangle.png</href></Icon></IconStyle></Style>\n";
  for (const KMLPeakStyle& s : peakStyles) {
	  mKml += "<Style id=\"peak_" + s.styleName + "\"><IconStyle>";
	  mKml += "<color>" + s.color + "</color>";
	  mKml += "<scale>" + std::to_string(s.scale) + "</scale>";
	  mKml += "<Icon><href>http://maps.google.com/mapfiles/kml/shapes/triangle.png</href></Icon></IconStyle></Style>\n";
  }

  mKml += "<Style id=\"saddle\"><IconStyle><color>ff00aa55</color><scale>0.6</scale><Icon><href>http://maps.google.com/mapfiles/kml/shapes/donut.png</href></Icon></IconStyle></Style>\n";
  mKml += "<Style id=\"basinsaddle\"><IconStyle><color>ffffaa00</color><scale>0.6</scale><Icon><href>http://maps.google.com/mapfiles/kml/shapes/donut.png</href></Icon></IconStyle></Style>\n";
  mKml += "<Style id=\"edge\"><LineStyle><color>ff0000ff</color><width>2</width></LineStyle></Style>\n";

  mKml += "<Style id=\"runoff\"><IconStyle><Icon><href>http://maps.google.com/mapfiles/kml/shapes/info_circle.png</href></Icon></IconStyle>\n";
  mKml += "<LineStyle><color>ff800000</color></LineStyle></Style>\n";
}

void KMLWriter2::startFolder(const std::string &name) {
  mKml += "<Folder><name>" + name + "</name>";
}

void KMLWriter2::endFolder() {
  mKml += "</Folder>";
}
  
void KMLWriter2::addPeak(const Peak &peak, const std::string& style, const std::string &name, const std::string& description, int id) {
  LatLng peakpos = mCoords.getLatLng(peak.location);
  if (id > 0)  mKml += "<Placemark id=\"peak_" + std::to_string(id) + "\">";
  else         mKml += "<Placemark>";
  mKml += "<styleUrl>#peak";
  if (style.length() > 0) mKml += "_" + style;
  mKml += "</styleUrl><Point><coordinates>\n";
  mKml += std::to_string(peakpos.longitude());
  mKml += ",";
  mKml += std::to_string(peakpos.latitude());
  mKml += ",";
  mKml += std::to_string(peak.elevation);
  mKml += "\n";
  mKml += "</coordinates></Point>";
  mKml += "<name>";
  mKml += name;
  mKml += "</name>";
  mKml += "<description>";
  mKml += description;
  mKml += "</description></Placemark>\n";
}

void KMLWriter2::addRunoff(const Runoff &runoff, const std::string &name) {
  LatLng pos = mCoords.getLatLng(runoff.location);
  mKml += "<Placemark><styleUrl>#runoff</styleUrl><Point><coordinates>\n";
  mKml += std::to_string(pos.longitude());
  mKml += ",";
  mKml += std::to_string(pos.latitude());
  mKml += ",";
  mKml += std::to_string(runoff.elevation);
  mKml += "\n";
  mKml += "</coordinates></Point>";
  mKml += "<name>";
  mKml += name;
  mKml += "</name>";
  mKml += "</Placemark>\n";
}

void KMLWriter2::addPromSaddle(const Saddle &saddle, const std::string &name, int id) {
  addSaddle(saddle, "#saddle", name, id);
}

void KMLWriter2::addBasinSaddle(const Saddle &saddle, const std::string &name) {
  addSaddle(saddle, "#basinsaddle", name, -1);
}

void KMLWriter2::addGraphEdge(const Peak &peak1, const Peak &peak2, const Saddle &saddle) {
  LatLng peak1pos = mCoords.getLatLng(peak1.location);
  LatLng saddlepos = mCoords.getLatLng(saddle.location);
  LatLng peak2pos = mCoords.getLatLng(peak2.location);

  mKml += "<Placemark><styleUrl>#edge</styleUrl><LineString><coordinates>\n";
  mKml += std::to_string(peak1pos.longitude());
  mKml += ",";
  mKml += std::to_string(peak1pos.latitude());
  mKml += ",";
  mKml += std::to_string(peak1.elevation);
  mKml += "\n";
  mKml += std::to_string(saddlepos.longitude());
  mKml += ",";
  mKml += std::to_string(saddlepos.latitude());
  mKml += ",";
  mKml += std::to_string(saddle.elevation);
  mKml += "\n";
  mKml += std::to_string(peak2pos.longitude());
  mKml += ",";
  mKml += std::to_string(peak2pos.latitude());
  mKml += ",";
  mKml += std::to_string(peak2.elevation);
  mKml += "\n";
  mKml += "</coordinates></LineString></Placemark>\n";
}

void KMLWriter2::addRunoffEdge(const Peak &peak, const Runoff &runoff) {
  LatLng peakpos = mCoords.getLatLng(peak.location);
  LatLng runoffpos = mCoords.getLatLng(runoff.location);
  
  mKml += "<Placemark><styleUrl>#runoff</styleUrl><LineString><coordinates>\n";
  mKml += std::to_string(peakpos.longitude());
  mKml += ",";
  mKml += std::to_string(peakpos.latitude());
  mKml += ",";
  mKml += std::to_string(peak.elevation);
  mKml += "\n";
  mKml += std::to_string(runoffpos.longitude());
  mKml += ",";
  mKml += std::to_string(runoffpos.latitude());
  mKml += ",";
  mKml += std::to_string(runoff.elevation);
  mKml += "\n";
  mKml += "</coordinates></LineString></Placemark>\n";
}

void KMLWriter2::addPeakEdge(const Peak &peak1, const Peak &peak2) {
  LatLng peak1pos = mCoords.getLatLng(peak1.location);
  LatLng peak2pos = mCoords.getLatLng(peak2.location);
  
  mKml += "<Placemark><LineString><coordinates>\n";
  mKml += std::to_string(peak1pos.longitude());
  mKml += ",";
  mKml += std::to_string(peak1pos.latitude());
  mKml += ",";
  mKml += std::to_string(peak1.elevation);
  mKml += "\n";
  mKml += std::to_string(peak2pos.longitude());
  mKml += ",";
  mKml += std::to_string(peak2pos.latitude());
  mKml += ",";
  mKml += std::to_string(peak2.elevation);
  mKml += "\n";
  mKml += "</coordinates></LineString></Placemark>\n";
}

string KMLWriter2::finish() {
  mKml += "</Document></kml>\n";
  return mKml;
}

void KMLWriter2::addSaddle(const Saddle &saddle, const char *styleUrl, const string &name, int id) {
  LatLng saddlepos = mCoords.getLatLng(saddle.location);
  if (id > 0)  mKml += "<Placemark id=\"saddle_" + std::to_string(id) + "\">";
  else         mKml += "<Placemark>";
  mKml += "<styleUrl>";
  mKml += styleUrl;
  mKml += "</styleUrl><Point><coordinates>\n";
  mKml += std::to_string(saddlepos.longitude());
  mKml += ",";
  mKml += std::to_string(saddlepos.latitude());
  mKml += ",";
  mKml += std::to_string(saddle.elevation);
  mKml += "\n";
  mKml += "</coordinates></Point>";
  mKml += "<name>";
  mKml += name;
  mKml += "</name>";
  mKml += "</Placemark>\n";
}
