/*
  FileDB.h - Library for creating and managing file-based databases.
  Created by Nicholas Pitt, February 26, 2015.
  Released under the LGPL license.
*/

#ifndef FileDB_h
#define FileDB_h

#include "Arduino.h"
#include "SD.h"

struct Entry {
  bool used;
};

template <class T>
class FileDB {
public:
  typedef bool Query(T &database_entry, T &query_entry);
  typedef void List(T &database_entry, void *data, size_t index);

  FileDB(size_t database_size) : database_size_(database_size) {};
  bool open(char *file_path) {
    file_ = SD.open(file_path, FILE_WRITE);
    if (file_.size() != database_size_ * sizeof(T)) {
      file_.close();
      file_ = SD.open(file_path, FILE_WRITE | O_TRUNC);
      for (size_t i = 0; i < database_size_; i++) {
        for (size_t j = 0; j < sizeof(T); j++) {
          file_.write((byte)'\0');
        }
        file_.flush();
      }
      return false;
    }
    return true;
  }
  void close() { file_.close(); }
  bool add(T &query_entry, Query &query) {
    T entry;

    file_.seek(0);
    for (size_t i = 0; i < database_size_; i++) {
      next(entry);
      if (!entry.used || query(entry, query_entry)) {
        file_.seek(i * sizeof(T));
        query_entry.used = true;
        file_.write(raw(query_entry), sizeof(T));
        file_.flush();
        return true;
      }
    }
    return false;
  }
  bool remove(T &query_entry, Query &query) {
    T entry;

    file_.seek(0);
    for (size_t i = 0; i < database_size_; i++) {
      next(entry);
      if (entry.used && query(entry, query_entry)) {
        file_.seek(i * sizeof(T));
        entry.used = false;
        file_.write(raw(entry), sizeof(T));
        file_.flush();
        return true;
      }
    }
    return false;
  }
  bool get(T &query_entry, Query &query) {
    T entry;

    file_.seek(0);
    for (size_t i = 0; i < database_size_; i++) {
      next(entry);
      if (entry.used && query(entry, query_entry)) {
        query_entry = entry;
        return true;
      }
    }
    return false;
  }
  void list(void *data, List &list) {
    size_t index = 0;
    T entry;

    file_.seek(0);
    for (int i = 0; i < database_size_; i++) {
      next(entry);
      if (entry.used) {
        list(entry, data, index++);
      }
    }
  }
  size_t count() {
    size_t count = 0;
    T entry;

    file_.seek(0);
    for (size_t i = 0; i < database_size_; i++) {
      next(entry);
      if (entry.used) {
        count++;
      }
    }
    return count;
  }

private:
  const size_t database_size_;
  File file_;

  byte *raw(T &entry) { return (byte *)(&entry); }
  void next(T &entry) {
    byte *raw_entry = raw(entry);

    for (size_t i = 0; i < sizeof(T); i++) {
      raw_entry[i] = file_.read();
    }
  }
};

#endif