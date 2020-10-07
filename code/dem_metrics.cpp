/*
 * MIT License
 *
 * Copyright (c) 2020 Oscar Argudo, based on prominence.cpp by Andrew Kirmse
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

#include "filter.h"
#include "peakbagger_collection.h"
#include "point_map.h"
#include "tile.h"
#include "tree_builder.h"
#include "divide_tree.h"
#include "island_tree.h"
#include "isolation_finder.h"

#include "easylogging++.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef PLATFORM_LINUX
#include <unistd.h>
#endif
#ifdef PLATFORM_WINDOWS
#include "getopt-win.h"
#endif
#include <set>

using std::set;
using std::string;
using std::vector;

INITIALIZE_EASYLOGGINGPP

static void usage() {
	printf("Usage:\n");
	printf("  dem_metrics demFile\n");
	printf("  where demFile is the DEM we want to process \n");
	printf("\n");
	printf("  Options:\n");
	printf("  -o directory      Directory for output data\n");
	printf("  -f format         \"ASC\", \"SRTM\", \"NED13-ZIP\", \"NED1-ZIP\" input files\n");
	printf("  -m min_prominence Minimum prominence threshold for output (in feet), default = 300ft\n");
	printf("  -t                Finalize output tree: delete all runoffs and then prune\n");
	exit(1);
}

int main(int argc, char** argv) {

	string output_directory(".");

	float minProminence = 300;
	FileFormat fileFormat = FileFormat::ASC;
	bool finalize = false;

	// Parse options
	START_EASYLOGGINGPP(argc, argv);
	int ch;
	string str;
	while ((ch = getopt(argc, argv, "f:m:o:t")) != -1) {
		switch (ch) {
		case 'f':
			str = optarg;
			if (str == "SRTM") {
				fileFormat = FileFormat::HGT;
			}
			else if (str == "NED1-ZIP") {
				fileFormat = FileFormat::NED1_ZIP;
			}
			else if (str == "NED13-ZIP") {
				fileFormat = FileFormat::NED13_ZIP;
			}
			else if (str == "ASC") {
				fileFormat = FileFormat::ASC;
			}
			else {
				printf("Unknown file format %s\n", optarg);
				usage();
			}
			break;

		case 'm':
			minProminence = static_cast<float>(atof(optarg));
			break;

		case 'o':
			output_directory = optarg;
			break;

		case 't':
			finalize = true;
			break;
		}
	}

	argc -= optind;
	argv += optind;

	// dem file to read
	if (argc < 1) {
		usage();
	}
	string demFilename(argv[0]);
	string demName = demFilename.substr(0, demFilename.find_last_of("."));

	std::cout << "Processing " << demFilename << std::endl;
	std::cout << "  prominence > " << minProminence << " feet" << std::endl;
	if (finalize) std::cout << "  finalize tree" << std::endl;


	// code adapted from ProminenceTask. 
	// We only process one DEM, so no cache, threads pool, etc.

	// Load the main tile manually
	Tile* tile = Tile::loadFromASCFile(demFilename);
	if (tile == nullptr) {
		VLOG(2) << "Couldn't load DEM";
		return -1;
	}

	// Build divide tree
	TreeBuilder* builder = new TreeBuilder(tile);
	DivideTree* divideTree = builder->buildDivideTree();
	delete builder;

	// Write full divide tree
	if (!divideTree->writeToFile(demName + "-divide_tree.dvt")) {
		std::cerr << "Failed to save divide tree file" << std::endl;
	}
	FILE* file = fopen((demName + "-divide_tree.kml").c_str(), "wb");
	if (file == nullptr) {
		delete tile;
		return -1;
	}
	fprintf(file, "%s", divideTree->getAsKml().c_str());
	fclose(file);

	// Prune low-prominence peaks to reduce divide tree size.  Rebuild island tree.
	IslandTree islandTree(*divideTree);
	islandTree.build();
	// finalize tree?
	if (finalize) {
		divideTree->deleteRunoffs();  // Does not affect island tree
	}
	// prune
	divideTree->prune(int(minProminence), islandTree);

	// Write pruned divide tree
	if (!divideTree->writeToFile(demName + "-divide_tree_pruned_" +
		std::to_string(int(minProminence)) + ".dvt")) {
		std::cerr << "Failed to save pruned divide tree file";
	}
	file = fopen((demName + "-divide_tree_pruned_" + std::to_string(int(minProminence))
		+ ".kml").c_str(), "wb");
	if (file == nullptr) {
		delete tile;
		return -1;
	}
	fprintf(file, "%s", divideTree->getAsKmlWithPopups().c_str());
	fclose(file);

	file = fopen((demName + "-divide_tree_pruned_" + std::to_string(int(minProminence))
		+ ".txt").c_str(), "wb");
	if (file == nullptr) {
		delete tile;
		return -1;
	}
	fprintf(file, "%s", divideTree->getAsText().c_str());
	fclose(file);

	// Build new island tree on pruned divide tree to get final prominence values
	IslandTree prunedIslandTree(*divideTree);
	prunedIslandTree.build();
	const CoordinateSystem& coords = divideTree->coordinateSystem();
	const vector<IslandTree::Node>& nodes = prunedIslandTree.nodes();
	const vector<Peak>& peaks = divideTree->peaks();
	const vector<Saddle>& saddles = divideTree->saddles();

	// Compute isolation of the peaks in the divide tree
	IsolationFinder isolFinder(nullptr, tile);
	vector<float> isolDistance(peaks.size(), -1);
	vector<LatLng> isolLocation(peaks.size());

	for (unsigned int i = 0; i < peaks.size(); i++) {
		Offsets offset = peaks[i].location;
		LatLng peak = tile->latlng(offset);

		IsolationRecord record = isolFinder.findIsolationInTile(offset);
		LatLng higher = record.closestHigherGround;
		if (record.foundHigherGround) {
			isolDistance[i] = peak.distanceEllipsoid(higher) / 1000; // kilometers
		}
		else {
			// -1 isolation to indicate no higher peak found
			isolDistance[i] = -1;
		}
		isolLocation[i] = higher;
	}

	// Write final metrics table
	string filename = demName + ".txt";
	file = fopen(filename.c_str(), "wb");
	fprintf(file, "latitude,longitude,elevation in feet,"
		"key saddle latitude,key saddle longitude,prominence in feet,"
		"isolation latitude,isolation longitude,isolation in km\n");
	for (int i = 1; i < (int)nodes.size(); ++i) {
		if (nodes[i].prominence >= minProminence) {
			const Peak& peak = peaks[i - 1];
			LatLng peakpos = coords.getLatLng(peak.location);
			LatLng colpos(0, 0);
			if (nodes[i].keySaddleId != IslandTree::Node::Null) {
				colpos = coords.getLatLng(saddles[nodes[i].keySaddleId - 1].location);
			}
			fprintf(file, "%.4f,%.4f,%d,%.4f,%.4f,%d,%.4f,%.4f,%.4f\n",
				peakpos.latitude(), peakpos.longitude(), peak.elevation,
				colpos.latitude(), colpos.longitude(),
				nodes[i].prominence,
				isolLocation[i - 1].latitude(), isolLocation[i - 1].longitude(),
				isolDistance[i - 1]);
		}
	}
	fclose(file);

	// cleanup
	delete divideTree;
	delete tile;

	return 0;
}
