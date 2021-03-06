/* extract means under polygons and,
visualize a class/segment map by:
coloring each class with its mean, from the imagery
inputs:
1) multispectral image (ENVI type 4)
2) co-registered label map (ENVI type 4)

outputs:
1 ) averaged product: same values over polygons
2) averages, by polygon index, for each polygon
*/

#include"misc.h"
int main(int argc, char ** argv){

  if(argc < 5){
    err("class_mean [input binary image file] [input binary class map file] [output image file name] [output spectra file name]");
  }

  // class file input
  str fn(argv[2]); // input file name
  cout << "input class file name:" << fn << endl;
  str hfn(hdr_fn(fn)); // auto-detect header file name
  cout << "header class file name:" << hfn << endl;
  size_t nrow, ncol, nband, np, i, j, k;
  hread(hfn, nrow, ncol, nband); // read header

  // imagery input
  str fn2(argv[1]); // input file name
  cout << "input data file name:" << fn2 << endl;
  str hfn2(hdr_fn(fn2)); // auto-detect header file name
  cout << "header data file name:" << hfn2 << endl;
  size_t nrow2, ncol2, nband2;
  hread(hfn2, nrow2, ncol2, nband2); // read header

  str ofn(argv[3]);
  if(exists(ofn)) err("output file already exists");

  str of2(argv[4]);
  if(exists(of2)) err("output spectra file already exists");

  np = nrow * ncol;
  float d;//r, g, b, h, s, v;
  if(nband != 1) err("this program defines results for 1-band images");

  if(nrow != nrow2 || ncol != ncol2){
    err("class map, and imagery, req'd to have same dimensions");
  }

  // read data into float array
  float * dat = bread(fn, nrow, ncol, nband);
  if(!dat) err("failed to read input class map");
  float * img = bread(fn2, nrow2, ncol2, nband2);
  if(!img) err("failed to read input img map");

  map<float, map<size_t, float>> avg; // class mean by class index, by band index
  map<float, map<size_t, size_t>> n_avg; // denominator of mean by class index, by band index

  // accumulate class data
  map<float, size_t> count;
  for0(i, np){
    float class_i = dat[i];
    if(count.count(class_i) < 1){
      count[class_i] = 0;
    }
    count[class_i] += 1;
    if(avg.count(class_i) < 1){
      avg[class_i] = map<size_t, float>();
      n_avg[class_i] = map<size_t, size_t>();
    }
    for0(k, nband2){
      if(avg[class_i].count(k) < 1){
        avg[class_i][k] = 0.;
        n_avg[class_i][k] = 0;
      }
      avg[class_i][k] += img[(np * k) + i];
      n_avg[class_i][k] ++;
    }
  }

  // divide total by denominator to get mean
  map<float, size_t>::iterator it;
  for(it = count.begin(); it != count.end(); it++){
    //simpler to write out than map<float, map<size_t, float>>::iterator
    float class_i = it->first;
    for0(k, nband2){
      avg[class_i][k] /= (float)n_avg[class_i][k];
    }
  }

  // number of codes: count.size(). write output image header:
  str ohfn(hdr_fn(ofn, true));
  cout << "output header file name: " << ohfn << endl;
  hwrite(ohfn, nrow2, ncol2, nband2); // rgb file: 3 bands

  // write colour encoded output: classes are coloured by class means, wrt the image
  FILE * outf = fopen(ofn.c_str(), "wb");
  for0(k, nband2){
    for0(i, np){
      d = avg[dat[i]][k];
      fwrite(&d, sizeof(float), 1, outf);
    }
  }
  fclose(outf);

  // write spectra to CSV file
  FILE * outf2 = fopen(of2.c_str(), "wb"); // should output band names to spectra

  size_t ci = 0;
  for(it = count.begin(); it != count.end(); it++){
    float class_i = it->first;
    if(ci > 0){
      fprintf(outf2, "\n");
    }
    fprintf(outf2, "%zu", class_i);
    for0(k, nband2){
      fprintf(outf2, ",%e", avg[class_i][k]);
    }
  }

  fclose(outf2);
  return 0;
}