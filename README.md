# Prominence and isolation

C++ programs for calculating the isolation and prominence from digital
elevation data.

## Building the code

C++11 support is required to build the code.  Binaries are placed in
the "debug" or "release" subdirectories.

### OSX and gcc

Debug version:

```
make  
```

Release version:

```
RELEASE=1 make
```

This has been tested under Mac OS 10.15 with clang 1100.0.33.12, and Unbuntu 16.04 with gcc 5.4.

### Windows

Debug version:

```
nmake -f makefile.win
```

Release version:

```
nmake RELEASE=1 -f makefile.win
```

This has been tested under Windows 10 with Microsoft Visual Studio 2017.

## Running the code

### Source data

Download source DEM data for the region of interest. 90-meter data is
available from NASA (SRTM), or improved void-filled data is at
[viewfinderpanoramas](http://viewfinderpanoramas.org/dem3.html).
Higher resolution data for North America is available at [The National
Map](https://viewer.nationalmap.gov/).

Note that SRTM filenames reference the southwest corner of the tile,
while NED filenames reference the northwest corner.


### Single Dem Metrics

Load any DEM of any size encoded as ASCII Grid (you can export .ASC from QGIS, for example).

```
dem_metrics.exe demFile

Options:
  -f format         Data format, choose ASC for ASCII Grid
  -m min_prominence Minimum prominence threshold for output, default = 300ft
  -o directory      Directory for output data
  -t                Finalize (terminate) output tree: delete all runoffs and then prune
```

This will generate, apart from the .dvt and .kml files, a txt file summarizing all the peaks above the prominence threshold. First row is header (see below) and then one row per peak follows.
```
latitude,longitude,elevation in feet,key saddle latitude,key saddle longitude,prominence in feet,isolation latitude,isolation longitude,isolation in km
```

This program also exports the Divide Tree as a TXT including the lat/lng coordinates, easier to digest from other applications.
The format is:
```
Peaks num_peaks
[... num_peaks point lines]
PromSaddles num_promSaddles
[... num_promSaddles point lines]
BasinSaddles num_basinSaddles
[... num_basinSaddles point lines]
Runoffs num_runoffs
[... num_runoffs point lines]
Edges num_edges
peak_id peak_id saddle_id
[...]
RunoffEdges
peak_id runoff_id
```

Each point line contains the following information (for peaks, saddles, runoffs):
```
id latitude longitude elevation_in_feet coord_x_in_dem coord_y_in_dem
```


### Isolation

```
isolation -- <min latitude> <max latitude> <min longitude> <max longitude>

Options:
  -i directory     Directory with terrain data
  -m min_isolation Minimum isolation threshold for output, default = 1km
  -o directory     Directory for output data
  -t num_threads   Number of threads, default = 1

```

This will generate one output text file per input tile, containing the
isolation of peaks in that tile.  The files can be merged and sorted
with standard command-line utilities.

### Prominence

First, generate divide trees for tiles of interest:

```
prominence -- <min latitude> <max latitude> <min longitude> <max longitude>

  Options:
  -i directory      Directory with terrain data
  -o directory      Directory for output data
  -f format         "SRTM", "NED13-ZIP", "NED1-ZIP" input files
  -k filename       File with KML polygon to filter input tiles
  -m min_prominence Minimum prominence threshold for output, default = 300ft
  -t num_threads    Number of threads, default = 1
```

This will produce divide trees with the .dvt extension, and KML files
that can be viewed in Google Earth.  The unpruned trees are generally
too large to be merged or to load into Earth.  Use the pruned versions
(identified by "pruned" in their filenames).

Next, merge the resultant divide trees into a single, larger divide
tree.  If there are thousands of input files, it will be much faster
to do this in multiple stages.

```
merge_divide_trees output_file_prefix input_file [...]
  Input file should have .dvt extension
  Output file prefix should have no extension

  Options:
  -f                Finalize output tree: delete all runoffs and then prune
  -m min_prominence Minimum prominence threshold for output, default = 300ft
```

The output is a dvt file with the merged divide tree, and a text file
with prominence values.  If desired, the text file can be filtered to
exclude peaks outside of a polygon specified in KML, for example, to
restrict the output to a single continent:

```
filter_points input_file polygon_file output_file
  Filters input_file, printing only lines inside the polygon in polygon_file to output_file

  Options:
  -a longitude    Add 360 to any longitudes < this value in polygon.
                  Allows polygon to cross antimeridian (negative longitude gets +360)
```


## Results

The isolation file has one peak per line, in this format:

latitude,longitude,elevation in feet,ILP latitude,ILP longitude,isolation in km

where ILP means isolation limit point.

A zip file with our isolation results for the world is [here](https://drive.google.com/file/d/0B3icWNhBosDXRm1pak56blp1RGc/view?usp=sharing).

The prominence file has one peak per line, in this format:

latitude,longitude,elevation in feet,key saddle latitude,key saddle longitude,prominence in feet

A zip file with our prominence results for the world is [here](https://drive.google.com/file/d/0B3icWNhBosDXZmlEWldSLWVGOE0/view?usp=sharing).

## Anti-prominence

The "anti-prominence" of low points can be computed by the same algorithm, simply by changing
the sign of the elevation values.  This can be done by giving the -a option to the 
"prominence" command.  Then, at the final stage of merging (with the -f flag), add the -a option 
again to flip the elevation values back to positive.

## More information

Explanations of what these calculations are about are at
http://andrewkirmse.com/isolation and
http://andrewkirmse.com/prominence, including nice visualizations.

This work was later published in the October, 2017 issue of the
journal "Progress of Physical Geography"
[here](https://doi.org/10.1177/0309133317738163).  The article can be
read [here](http://www.andrewkirmse.com/prominence-article).
