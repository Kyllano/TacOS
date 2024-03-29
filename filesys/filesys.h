/*! \file filesys.h
    \brief Data structures to represent the Nachos file system.

        A file system is a set of files stored on disk, organized
        into directories.  Operations on the file system have to
        do with "naming" -- creating, opening, and deleting files,
        given a textual file name.  Operations on an individual
        "open" file (read, write, close) are to be found in the OpenFile
        class (openfile.h).

        There are two key data
        structures used in the file system.  There is a single "root"
        directory, listing all of the files in the file system; unlike
        UNIX, the baseline system does not provide a hierarchical
        directory structure.  In addition, there is a bitmap for
        allocating disk sectors.  Both the root directory and the
        bitmap are themselves stored as files in the Nachos file
        system -- this causes an interesting bootstrap problem when
        the simulated disk is initialized.

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

#ifndef FS_H
#define FS_H

/*! Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number
// of files that can be loaded onto the disk.
*/
#define FreeMapFileSize (NUM_SECTORS / BITS_IN_BYTE)

#include "filesys/openfile.h"
#include "kernel/copyright.h"

int FindDir(char *);
/*! \brief Defines the Nachos file system
 */
class FileSystem {
public:
  FileSystem(bool format);   //!< Initialize the file system.
                             //!< Must be called *after* "synchDisk"
                             //!< has been initialized.
  ~FileSystem();   //!< Destructor of file system. Called when Nachos is
                   //!< shut-down

  int Create(char *name, int initialSize);
  //!< Create a file (UNIX creat)

  OpenFile *Open(char *name);   //!< Open a file (UNIX open)

  int Remove(char *name);   //!< Delete a file (UNIX unlink)

  void List();   //!< List all the files in the file system

  void Print();   //!< List all the files and their contents

  OpenFile *GetFreeMapFile();   //!< Get the free map table

  OpenFile *GetDirFile();   //!< Get the root directory

  int Mkdir(char *);   //!< Create a new directory

  int Rmdir(char *);   //!< Delete a directory

private:
  OpenFile *freeMapFile;   /*!< Bit map of free disk blocks,
                            represented as a file
                           */
  OpenFile *directoryFile; /*!< "Root" directory -- list of
                            file names, represented as a file
                            */
};

#endif   // FS_H
