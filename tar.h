/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TAR__
#define __TAR__

#include <string>
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#if !defined(__APPLE__)
#include <sys/sysmacros.h>
#endif
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_DIR_MODE S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH // 0755

#define BLOCKSIZE       512
#define BLOCKING_FACTOR 20
#define RECORDSIZE      10240

// file type values (1 octet)
#define REGULAR          0
#define NORMAL          '0'
#define HARDLINK        '1'
#define SYMLINK         '2'
#define CHAR            '3'
#define BLOCK           '4'
#define DIRECTORY       '5'
#define FIFO            '6'
#define CONTIGUOUS      '7'

// tar entry metadata structure (singly-linked list)
typedef struct tar_t {
    char original_name[100];                // original filenme; only availible when writing into a tar
    unsigned int begin;                     // location of data in file (including metadata)
    union {
        union {
            // Pre-POSIX.1-1988 format
            struct {
                char name[100];             // file name
                char mode[8];               // permissions
                char uid[8];                // user id (octal)
                char gid[8];                // group id (octal)
                char size[12];              // size (octal)
                char mtime[12];             // modification time (octal)
                char check[8];              // sum of unsigned characters in block, with spaces in the check field while calculation is done (octal)
                char link;                  // link indicator
                char link_name[100];        // name of linked file
            };

            // UStar format (POSIX IEEE P1003.1)
            struct {
                char old[156];              // first 156 octets of Pre-POSIX.1-1988 format
                char type;                  // file type
                char also_link_name[100];   // name of linked file
                char ustar[8];              // ustar\000
                char owner[32];             // user name (string)
                char group[32];             // group name (string)
                char major[8];              // device major number
                char minor[8];              // device minor number
                char prefix[155];
            };
        };

        char block[512];                    // raw memory (500 octets of actual data, padded to 1 block)
    };

    struct tar_t * next;
}tar_t;

// core functions //////////////////////////////////////////////////////////////
// read a tar file
// archive should be address to null pointer
int tar_read(const int fd, struct tar_t ** archive, const char verbosity);

// write to a tar file
// if archive contains data, the new data will be appended to the back of the file (terminating blocks will be rewritten)
int tar_write(const int fd, struct tar_t ** archive, const size_t filecount, const char * files[], const char verbosity);

// recursive freeing of entries
void tar_free(struct tar_t * archive);
// /////////////////////////////////////////////////////////////////////////////

// utilities ///////////////////////////////////////////////////////////////////
// print contents of archive
// verbosity should be greater than 0
int tar_ls(FILE * f, struct tar_t * archive, const size_t filecount, const char * files[], const char verbosity);

// extracts files from an archive
int tar_extract(const int fd, struct tar_t * archive, const size_t filecount, const char * files[], const char verbosity);

// update files in tar with provided list
int tar_update(const int fd, struct tar_t ** archive, const size_t filecount, const char * files[], const char verbosity);

// remove entries from tar
int tar_remove(const int fd, struct tar_t ** archive, const size_t filecount, const char * files[], const char verbosity);

// show files that are missing from the current directory
int tar_diff(FILE * f, struct tar_t * archive, const char verbosity);
// /////////////////////////////////////////////////////////////////////////////

// internal functions; generally don't call from outside ///////////////////////
// print raw data with definitions (meant for debugging)
int print_entry_metadata(FILE * f, struct tar_t * entry);

// print metadata of entire tar file
int print_tar_metadata(FILE * f, struct tar_t * archive);

// check if file with original name/modified name exists
struct tar_t * exists(struct tar_t * archive, const char * filename, const char ori);

// read file and construct metadata
int format_tar_data(struct tar_t * entry, const char * filename, const char verbosity);

// calculate checksum (6 ASCII octet digits + NULL + space)
unsigned int calculate_checksum(struct tar_t * entry);

// print single entry
// verbosity should be greater than 0
int ls_entry(FILE * f, struct tar_t * archive, const size_t filecount, const char * files[], const char verbosity);

// extracts a single entry
// expects file descriptor offset to already be set to correct location
int extract_entry(const int fd, struct tar_t * entry, const char verbosity, std::string customUnTarPath);

// write entries to a tar file
int write_entries(const int fd, struct tar_t ** archive, struct tar_t ** head, const size_t filecount, const char * files[], int * offset, const char verbosity);

// add ending data
int write_end_data(const int fd, int size, const char verbosity);

// check if entry is a match for any of the given file names
// returns index + 1 if match is found
int check_match(struct tar_t * entry, const size_t filecount, const char * files[]);
// /////////////////////////////////////////////////////////////////////////////

void set_rootPath(std::string Path);

int Tar(char * filename, char** path, int fileCount);

int UnTar(char * filename, char** path, int fileCount);

int AppenderTar(char * filename, char** path, int fileCount);

int RemoveTar(char * filename, char** path, int fileCount);

int UpdateTar(char * filename, char** path, int fileCount);

int ExtractTar(char * filename, char** path, int fileCount);

#endif
