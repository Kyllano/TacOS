/*! \file  directory.h
    \brief Data structures to manage a UNIX-like directory of file names.

        A directory is a table of pairs: <file name, sector #>,
        giving the name of each file in the directory, and
        where to find its file header (the data structure describing
        where to find the file's data blocks) on disk.

       We assume mutual exclusion is provided by the caller.

 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------

*/

#include "kernel/copyright.h"

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "filesys/openfile.h"
#define FILENAMEMAXLEN 80

/*! \brief Defines a directory entry
//
//  Each entry gives the name of the file, and where
// the file's header is to be found on disk.
// Internal data structures kept public so that Directory operations can
// access them directly.
*/

class DirectoryEntry {
public:
  bool inUse;                    //!< Is this directory entry in use?
  int sector;                    /*!< Location on disk to find the
                                      FileHeader for this file
                                 */
  char name[FILENAMEMAXLEN + 1]; /*!< Text name for file, with +1 for
                                      the trailing '\0'
                                 */
};

/*!\brief Defines a UNIX-like "directory".
//
// Each entry in the directory describes a file, and where to find
// it on disk.
// The directory data structure can be stored in memory, or on disk.
// When it is on disk, it is stored as a regular Nachos file.
// The constructor initializes a directory structure in memory; the
// FetchFrom/WriteBack operations shuffle the directory information
// from/to disk.
*/

class Directory {
public:
  Directory(int size);   // Initialize an empty directory
                         // with space for "size" files

  ~Directory();   // De-allocate the directory

  void FetchFrom(OpenFile *file);   // Init directory contents from disk
  void WriteBack(OpenFile *file);   // Write modifications to
                                    // directory contents back to disk

  int Find(char *name);   // Find the sector number of the
                          // FileHeader for file: "name"

  int Add(char *name, int newSector);   // Add a file name into the directory

  int Remove(char *name);   // Remove a file from the directory

  void List(char *, int);   // Print the names of all the files
                            // in the directory

  void Print();   // Verbose print of the contents
                  //  of the directory -- all the file
                  //   names and their contents.
  bool empty();

private:
  int tableSize;               //!< Number of directory entries
  DirectoryEntry *table;       /*!< Table of pairs:
                                  <file name, file header location>
                               */
  int FindIndex(char *name);   // Find the index into the directory
                               //   table corresponding to "name"
};

#endif   // DIRECTORY_H
