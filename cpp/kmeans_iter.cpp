// 20200125 kmeans implementation with arbitrary labels input
#include"misc.h" // do NAN tolerance by not putting the NAN labels into a bucket!

class labelled_bucket; // implement this?
size_t nrow, ncol, nband, np, i, j, k, n, iter_max, K, nmf; // variables
float * dat, *seed;
// float * means, * dmin, * dmax,
float *means_color, *label, *buckets, *update, tol;
size_t n_change;

map<float, list<size_t>> points; // lists of datapoint indices, organized by label
map<float, float * > means; // means for each label

void find_nearest(size_t b){
  // b bucket index.
  int K = means.size();
  float bucket_c = buckets[b]; // for all points in this bucket, find nearest mean

  // find the nearest centre, for data point with index: i
  // put a loop in this one to find nearest neighbour of a chunk of points..
  size_t i, j, k;
  float d, x, c, *mean;

  list<size_t> * p = &points[bucket_c]; // list the points in this class to iterate

  // for each point in bucket..
  for(list<size_t>::iterator it = p->begin(); it != p->end(); it++){
    i = *it;

    float nearest_c = NAN;
    float nearest_d = FLT_MAX;

    for0(j, K){
      c = buckets[j]; // float c = ti->first; // class label we're comparing against
      mean = means[c]; // vector representing the present mean

      d = 0.; // distance from this point to centre

      for0(k, nband){
        x = dat[np * k + i] - mean[k]; // compare point to mean each band
        d += x * x;
      }
      d = sqrt(d);
      if(isnan(d) || isinf(d)){
      }
      else{
        if(d < nearest_d){
          nearest_d = d;
          nearest_c = c;
        }
      }
    }
    update[i] = nearest_c;
    // if(nearest_c == NAN || isnan(nearest_c)) { printf("QUIT\n"); exit(1); }
  }
}

int main(int argc, char ** argv){
  printf("--------- argc %d\n", argc);
  for(int i = 0; i < argc; i++) printf(" %d %s\n", i, argv[i]);
  float d; // temporary data variable
  iter_max = 100; // default max iterations

  if(argc < 3) cout << "kmeans [input binary image file name] [input label file name] [percent tolerance] " << endl;
  // << " [optional parameter: float value: add random seed with label!] # default to random initialization # NAN = no label. Remember to scale data first!!" << endl;

  str fn("stack.bin");
  if(argc > 1) fn = str(argv[1]); // input image file name
  if(!exists(fn)) err(str("failed to open input file: ") + fn);

  str lfn("stack.bin_nearest_centre.bin");
  if(argc > 2) lfn = str(argv[2]);
  if(!exists(lfn)) err(str("failed to open input file: ") + lfn);

  if(argc > 3) tol = atof(argv[3]);
  else tol = 3.; // percent class change (%) tolerance
  if(tol < 0 || tol >= 100) err("tol must be between 0 and 100.");

  printf("tolerance: %f\n", tol);

  // K is not input in this version because it's prescribed by the seeding
  str hfn(hdr_fn(fn)); // input header file name
  hread(hfn, nrow, ncol, nband); // read header

  str lhfn(hdr_fn(lfn));
  size_t nrow2, ncol2, nband2; // read label file
  hread(lhfn, nrow2, ncol2, nband2);
  if(nrow2 != nrow || ncol2 != ncol || nband2 != 1){
    err(str("please check file: ") + str(lhfn));
  }
  printf("loadfloat\n");

  cout << "fn " << fn << endl;
  cout << "lfn" << lfn << endl;
  np = nrow * ncol; // number of input pix
  dat = bread(fn, nrow, ncol, nband); // load floats to array
  seed = bread(lfn, nrow, ncol, 1); // label seed array
  means_color = falloc(np * nband); // output nearest mean for visualization

  label = falloc(np); // one label per pixel; label is not defined if pix. index not appear in "points"
  for0(i, np) label[i] = NAN; // default to NAN
  update = falloc(np); // new set of label
  for0(i, np) update[i] = NAN;

  printf("seed\n");

  set<float> my_seeds;
  for0(i, np){
    d = seed[i];
    // my_seeds.insert(d); // label could be NaN without data being NaN!?
    if(!(isnan(d) || isinf(d)) ) my_seeds.insert(d);
  }

  K = my_seeds.size(); // if(add_random_seed) K ++;

  int * is_nan = (int *) alloc(sizeof(int) * np);

  for0(i, np){
    d = seed[i]; // use seed file to assign points to buckets
    if(isnan(d) || isinf(d)){
      is_nan[i] = 1;
    }
    else{ 
     is_nan[i] = 0;
     points[d].push_back(i); // map<float, list<size_t>> points; // lists of datapoint indices, organized by label
    }
  }

  size_t ci = 0;

  // add random seed at end. make sure it's not equal to any of the means. make sure it's label is higher than the highest label so far!
  printf("%d buckets\n", K);
  buckets = falloc(K);
  for(map<float, list<size_t>>::iterator it = points.begin(); it != points.end(); it++){
    float c = it->first;
    buckets[ci++] = c;
    means[c] = falloc(nband); // each mean needs the number of dimensions we have in our image
  }

  printf("buckets: ");
  for0(i, K){
    printf("\n,%f", buckets[i]);
    cout << points[buckets[i]];
    printf("\n");
  }
  if(true){
    str ofn(fn + str("_kmeans_iter_") + zero_pad(to_string(0), 4) + str(".bin")); // output class labels
    str ohn(fn + str("_kmeans_iter_") + zero_pad(to_string(0), 4) + str(".hdr"));
    hwrite(ohn, nrow, ncol, 1, 4); // write type 4 header
    bwrite(seed, ofn, nrow, ncol, 1); // write data
  }

  // start the algorithm
  for0(n, iter_max){
    printf("iter %zu\n", n);
    n_change = 0; // still more to parallelize here!

    for(map<float, list<size_t>>::iterator it = points.begin(); it != points.end(); it++){
      float c = it->first;
      for0(k, nband) means[c][k] = 0.; // start means to 0 each iteration
    }

    map<float, list<size_t>>::iterator it; // process the random seed separately?
    for(j = 0; j < K; j++){
      float c = buckets[j];
      // add the contribution of each point to the new means (should parallelize this)
      list<size_t>::iterator ti;
      list<size_t> * p = &points[c];
      for(ti = p->begin(); ti != p->end(); ti++){
        size_t i = *ti;
        //cout << "i"<<i<<endl;
        for0(k, nband){
          //cout <<"k"<<k<<endl;
          d = dat[np * k + i];
          //cout << "m" << " c " << c << " i " << i << endl;
          means[c][k] += d; //dat[np * k + i]; // means[(((size_t)label[i]) * nband) + k] += dat[(np * k) + i];
        }
      }
    }

    for0(j, K){
      float c = buckets[j]; // it->first;
      float n_pts = (float) points[c].size();
      for0(k, nband) means[c][k] /= n_pts;
    }

    for0(j, K){
      float c = buckets[j];
      printf(" c[%f] ", c);
      for0(k, nband) printf("%f,", means[c][k]);
      printf("\n");
    }

    // for0(i, K) if(dcount[i] > 0) for0(j, nband) mean[(i * nband) + j] /= dcount[i]; // mean = total / count

    size_t n_buckets = means.size();
    parfor(0, n_buckets, find_nearest); // find nearest centre to each point.. do this on per-bucket basis for now!!!!!

    for0(i, np) if(label[i] != update[i]) n_change ++; // find out how many pixels changed
    float pct_chg = 100. * (float)n_change / (float)np; // (float)n_good; // plot change info
    printf("iter %zu of %zu n_change %f\n", n + 1, iter_max, pct_chg);

    //for0(i, K) cout << buckets[i] << points[buckets[i]] << endl; // bucket contents

    float * tmp = label; // swap
    label = update;
    update = tmp;

    for0(i, np) update[i] = 0.; // close enough? stop iterating if <1% of pix changed class
    if(pct_chg < tol){
      printf("break\n");
      break;
    }
    if(true){
      str ofn(fn + str("_kmeans_iter_") + zero_pad(to_string(n+1), 4) + str(".bin")); // output class labels
      str ohn(fn + str("_kmeans_iter_") + zero_pad(to_string(n+1), 4) + str(".hdr"));
      hwrite(ohn, nrow, ncol, 1, 4); // write type 4 header
      bwrite(label, ofn, nrow, ncol, 1); // write data
    }
  }

  printf("write class labels..\n");
  str ofn(fn + str("_kmeans.bin")); // output class labels
  str ohn(fn + str("_kmeans.hdr"));
  printf("got here..\n"); 
  for0(i, np){
    if(is_nan[i]) label[i] = NAN;
  }
  cout << "+w " << ofn << endl;
  cout << "+w " << ohn << endl;
  bwrite(label, ofn, nrow, ncol, 1); // write data
  printf("got there..\n");
  hwrite(ohn, nrow, ncol, 1, 4); // write type 4 header
  /*
  str omn(fn + str("_means_color.bin")); // output class centres, for each pixel categorized
  str omh(fn + str("_means_color.hdr"));
  printf("now here..\n");
  for0(i, np){
    printf("i %zu\n", i);
    float c = label[i];
    cout << c << endl;
    cout << means[c]<<endl;

    if(isnan(c)) continue;
    for0(k, nband){
      d = means[c][k]; // ((size_t)label[i] * nband) + k];
      means_color[(k * np) + i] = d; // colour by mean
    }
  }
  printf("even here..\n");
  hwrite(omh, nrow, ncol, nband, 4); // write data
  bwrite(means_color, omn, nrow, ncol, nband);
  */

  free(dat);
  free(label);
  return 0;
}
