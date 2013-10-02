// Copyright 2013 Yangqing Jia
// This program converts a set of images to a leveldb by storing them as Datum
// proto buffers.
// Usage:
//    convert_dataset ROOTFOLDER LISTFILE DB_NAME
// where ROOTFOLDER is the root folder that holds all the images, and LISTFILE
// should be a list of files as well as their labels, in the format as
// subfolder1/file1.JPEG 0
// ....
// You are responsible for shuffling the files yourself.

#include <glog/logging.h>
#include <leveldb/db.h>

#include <string>
#include <iostream>
#include <fstream>

#include "caffe/proto/caffe.pb.h"
#include "caffe/util/io.hpp"

using namespace caffe;
using std::string;
using std::stringstream;

// A utility function to generate random strings
void GenerateRandomPrefix(const int n, string* key) {
  const char* kCHARS = "abcdefghijklmnopqrstuvwxyz";
  key->clear();
  for (int i = 0; i < n; ++i) {
    key->push_back(kCHARS[rand() % 26]);
  }
  key->push_back('_');
}

int main(int argc, char** argv) {
  ::google::InitGoogleLogging(argv[0]);
  std::ifstream infile(argv[2]);
  leveldb::DB* db;
  leveldb::Options options;
  options.error_if_exists = true;
  options.create_if_missing = true;
  LOG(INFO) << "Opening leveldb " << argv[3];
  leveldb::Status status = leveldb::DB::Open(
      options, argv[3], &db);
  CHECK(status.ok()) << "Failed to open leveldb " << argv[3];

  string root_folder(argv[1]);
  string filename;
  int label;
  Datum datum;
  int count = 0;
  char key_cstr[100];
  while (infile >> filename >> label) {
    ReadImageToDatum(root_folder + filename, label, &datum);
    sprintf(key_cstr, "%08d_%s", count, filename.c_str());
    string key(key_cstr);
    string value;
    // get the value
    datum.SerializeToString(&value);
    db->Put(leveldb::WriteOptions(), key, value);
    if (++count % 1000 == 0) {
      LOG(ERROR) << "Processed " << count << " files.";
    }
  }

  delete db;
  return 0;
}