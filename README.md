# bcws-psu-research
Image analysis research collaboration featuring:
* Digital Platforms and Data Division (DPDD), Office of The BC Chief Information Officer (OCIO) 
* BC Wildfire Service (BCWS) Predictive Services Unit (PSU)
* Thompson Rivers University (TRU)

## Sample results
### Interactive classification using BCWS FTL MVP software: end-to-end decision support system (DSS
<img src="output/20210127_mvp.png" width="1089" height="442">

### Forest change detection product using RCM
<img src="output/difference.gif" width="640">

## features
* High-contrast visualization for multispectral imagery and classification maps
* Supervised and unsupervised classification methods, for multispectral imagery
   * K-Means++
   * Hierarchical Agglomerative Clustering (HAC) Scikit-Learn
   * HAC: Daniel Müllner, fastcluster: Fast Hierarchical, Agglomerative Clustering Routines for R and Python, Journal of Statistical Software 53 (2013), no. 9, 1–18, URL http://www.jstatsoft.org/v53/i09/
   * HAC: bootstrappable, direct implementation (can initialize with HAC results on tiles)
* manipulation operations for class maps

To appear: more utilities for integrating open remotely-sensed imagery

## Collaborators and Contributors:
* Dana Hicks, BCWS
* Joanna Wand, BCWS
* Dr. Musfiq Rahman, TRU
* Dr. David Hill, TRU

### TRU Computing Science ML/AI Students Co-supervised:
* Gagan Bajwa
* Brad Crump
* Francesca Rammuno

## Alumni:
* Brady Holliday, BCWS
* Jabed Tomal, TRU

## requirements:
* python 2
* gnu g++

Tested on ubuntu 20

## how to:
0) Set up the programs:
    python setup.py
    
1) View sample input data:

    python py/read_multispectral.py data/mS2.bin
    
2) Run K-Means clustering on the sample data:

    python py/kmeans.py data/mS2.bin

3) Run Hierarchical Agglomerative Clustering (HAC) on sample data:

    python py/fast_cluster.py data/mS2.bin
 
4) Run HAC to produce tiled seeds:
    
    python py/fast_cluster_tiling.py data/mS2.bin 50 5 24.5
    
5) Run HAC with seeds produced by 4), allowing HAC to run on larger images

    bin/hclust data/mS2.bin data/mS2.bin_label.bin 10
    
    video of clustering: https://www.youtube.com/watch?v=ooOHEubDNvM
   


## License

Copyright 2020 Province of British Columbia

Licensed under the Apache License, Version 2.0 (the "License");
you mayn't use these files except in compliance with the License.

You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0

Unless req'd by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.

See the License for specific language governing permissions
and limitations under the License.
