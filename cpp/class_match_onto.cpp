/* by arichardson 20191023 identify a correspondence between classification maps */
#include"misc.h"
int main(int argc, char ** argv){
  if(argc < 3) err(str("class_match_onto ") +
  str("[input class file #1] ") +
  str("[class file #2 to match onto] ") +
  str("[output class file (#1 transformed to match classes of #2)]"));

  str fn1(argv[1]); // input file name
  str fn2(argv[2]); // input file name #2
  str ofn(argv[3]);
  if(exists(ofn)) err("output file already exists.");

  str hfn1(hdr_fn(fn1)); // auto-detect header filenames
  str hfn2(hdr_fn(fn2));

  size_t nrow, ncol, nband, nrow2, ncol2, nband2, np, i, j;
  hread(hfn1, nrow, ncol, nband); // read headers
  hread(hfn2, nrow2, ncol2, nband2);

  if(nrow != nrow2) err("nrow1 != nrow2"); // assert headers match
  if(ncol != ncol2) err("ncol1 != ncol2");
  if(nband != nband2) err("nband2 != nband2");

  np = nrow * ncol; // number of pixels
  if(nband != 1) err("this program defines results for 1-band images");

  // read data into float array
  float * dat1 = bread(fn1, nrow, ncol, nband);
  float * dat2 = bread(fn2, nrow, ncol, nband);

  // accumulate data from a in terms of b
  float a, b;
  map<float, map<float, size_t>> count;
  for0(i, np){
    a = dat1[i];
    b = dat2[i];
    if(count.count(a) < 1){
      count[a] = map<float, size_t>();
    }
    if(count[a].count(b) < 1){
      count[a][b] = 0;
    }
    count[a][b] += 1;
  }

  // sort the data to determine lookup
  float maxi;
  size_t c, maxc;
  map<float, float> lookup;
  map<float, size_t>::iterator it2;
  map<float, map<float, size_t>>::iterator it;

  // for each index to be mapped, create lookup
  for(it = count.begin(); it != count.end(); it++){
    maxc = 0;
    maxi = -1;
    a = it->first;
    cout << a << endl;
    // determine highest count
    // note: for qa and analysis, should also output inclusion mtx
    for(it2 = count[a].begin(); it2 != count[a].end(); it2++){
      c = it2->second;
      cout << "\t" << c << " --> " << it2->first << endl;
      if(maxi < 0. || c >= maxc){
        maxi = it2->first;
        maxc = c;
      }
    }
    cout << a << " --> " << maxi << endl << endl;
    lookup[a] = maxi; //maxc; //maxi; is it maxi or maxc?
  }

  // iterate the data and apply lookup
  float * dat = (float *) alloc(np * sizeof(float));
  for0(i, np) dat[i] = lookup[dat1[i]];

  map<float, size_t> count_b;
  for0(i, np){
    b = dat2[i];
    if(count_b.count(b) < 1) count_b[b] = 0;
    count_b[b] += 1;
  }

  i = 0;
  map<float, size_t>::iterator ti;
  for(ti = count_b.begin(); ti != count_b.end(); ti++){
    dat[i] = ti->first;
    cout << "\tforce value: " << i << " --> " << dat[i] << endl;
    i++;
  }

  FILE * f = fopen(ofn.c_str(), "wb");
  fwrite(dat, sizeof(float), np, f);
  fclose(f);

  hwrite(ofn + str(".hdr"), nrow, ncol, 1);

  free(dat);
  free(dat1);
  free(dat2);
  return 0;
}