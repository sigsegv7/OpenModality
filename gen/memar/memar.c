/*
 * Copyright (c) 2025 Ian Marco Moffett and the OpenModality engineers
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define _DEFAULT_SOURCE
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define MEMAR_MAGIC "LORD"
#define MEMAR_MAGIC_LEN 4
#define FILE_NAME_MAX 99

/* Align a value up to a nearest multiple */
#define ALIGN_UP(value, align) (((value) + (align)-1) & ~((align)-1))

/* File alignment */
#define FILE_ALIGN 8

/* Default output archive name */
#define DEFAULT_OUTPUT "initrd.mr"

static const char *output_file = DEFAULT_OUTPUT;
static const char *input_dir = NULL;
static char file_pad[FILE_ALIGN] = {0};

/*
 * Represents a header that sits on top of each file
 * in the archive
 *
 * @hdr_size: Length of the header
 * @name: Filename
 *
 * This format is as follows:
 *
 * < FILE HEADER   >
 * < FILE CONTENTS >
 * < PADDING >
 * ...
 */
struct __attribute__((packed)) file_hdr {
    char magic[MEMAR_MAGIC_LEN];
    size_t hdr_size;
    char name[FILE_NAME_MAX];
};

static void
help(void)
{
    printf(
        "memar - generate a memory archive\n"
        "Copyright (c) 2025 Ian Marco Moffett\n"
        "------------------------------------\n"
        "[-h]   Display this help menu\n"
        "[-i]   Input directory to generate from\n"
        "[-o]   Output archive file\n"
    );
}

/*
 * Initializes a file header and returns the length
 */
static size_t
file_hdr_init(const char *name, struct file_hdr *hdr)
{
    size_t name_len;

    if (name == NULL || hdr == NULL) {
        return 0;
    }

    /* Truncate name if needed */
    name_len = strlen(name);
    if (name_len >= FILE_NAME_MAX - 1) {
        name_len = FILE_NAME_MAX - 1;
    }

    memcpy(hdr->name, name, name_len);
    memcpy(hdr->magic, MEMAR_MAGIC, MEMAR_MAGIC_LEN);
    hdr->hdr_size = name_len;
    hdr->hdr_size += sizeof(struct file_hdr) - FILE_NAME_MAX;
    return hdr->hdr_size;
}

static void
write_file(int out_fd, const char *path)
{
    struct file_hdr hdr;
    void *file;
    size_t real_size, align_size;
    size_t hdr_size;
    uint8_t pad_len;
    int fd;

    if (path == NULL) {
        return;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("could not open \"%s\"\n", path);
        perror("open");
        return;
    }

    /* Obtain the file size */
    real_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    /* Attempt to map it */
    file = mmap(NULL, real_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file == NULL) {
        perror("mmap");
        close(fd);
        return;
    }

    /* Initialize the header */
    hdr_size = file_hdr_init(path, &hdr);

    /* Compute the length of padding */
    align_size = ALIGN_UP(real_size, FILE_ALIGN);
    pad_len = align_size - real_size;

    /* Write the file + padding */
    write(out_fd, &hdr, hdr_size);
    write(out_fd, file, real_size);
    if (pad_len > 0) {
        write(out_fd, file_pad, pad_len);
    }
    close(fd);
}

static void
concat_foreach(int dirfd, int out_fd, const char *path)
{
    int subdir_fd;
    char pathbuf[PATH_MAX];
    struct dirent *dirent;
    DIR *dir;

    if ((dir = fdopendir(dirfd)) == NULL) {
        perror("fdopendir");
        return;
    }

    /*
     * Recursively scan each file and directory within
     * the root.
     */
    while ((dirent = readdir(dir)) != NULL) {
        if (dirent->d_name[0] == '.') {
            continue;
        }

        switch (dirent->d_type) {
        case DT_DIR:
            snprintf(pathbuf, sizeof(pathbuf), "%s/%s", path, dirent->d_name);
            subdir_fd = open(pathbuf, O_RDONLY);
            if (subdir_fd < 0) {
                perror("open");
                return;
            }

            printf("[d] %s\n", pathbuf);
            concat_foreach(subdir_fd, out_fd, pathbuf);
            close(subdir_fd);
            break;
        case DT_REG:
            snprintf(pathbuf, sizeof(pathbuf), "%s/%s", path, dirent->d_name);
            printf("[f] %s\n", pathbuf);
            write_file(out_fd, pathbuf);
            break;
        }
    }

    closedir(dir);
}

/*
 * Concatenate each file within a directory into a single
 * output archive
 */
static void
dir_concat(void)
{
    struct stat statb;
    int fd, out_fd;

    fd = open(input_dir, O_RDONLY);
    if (fd < 0) {
        perror("open[input]");
        return;
    }

    /* Create the output file */
    out_fd = open(output_file, O_WRONLY | O_CREAT, 0666);
    if (out_fd < 0) {
        perror("open[output]");
        close(fd);
        return;
    }

    if (fstat(fd, &statb) < 0) {
        perror("fstat");
        return;
    }

    if (!S_ISDIR(statb.st_mode)) {
        printf("error: \"%s\" is not a directory\n", input_dir);
        return;
    }

    concat_foreach(fd, out_fd, input_dir);
    close(out_fd);
    close(fd);
}

int
main(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "hi:o:")) != -1) {
        switch (opt) {
        case 'h':
            help();
            return -1;
        case 'i':
            input_dir = strdup(optarg);
            break;
        case 'o':
            output_file = strdup(optarg);
            break;
        }
    }

    if (input_dir == NULL) {
        printf("error: expected input file\n");
        help();
        return -1;
    }

    dir_concat();
    return 0;
}
