#pragma once

// moved to solution config
//#define _CRT_SECURE_NO_WARNINGS

#if defined(__GNUC__)
// Ensure we get the 64-bit variants of the CRT's file I/O calls
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif
#endif

#include "miniz.h"

#include <string.h>
#include <stdio.h>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace zip {
	inline bool unzip(const char* filepath, const char* outputDirPath) {
		mz_zip_archive zipArchive;
		mz_bool status;
		memset(&zipArchive, 0, sizeof(zipArchive));
		status = mz_zip_reader_init_file(&zipArchive, filepath, 0);
		if (!status)
		{
			printf("mz_zip_reader_init_file() failed!\n");
			return EXIT_FAILURE;
		}

		int numFiles = (int)mz_zip_reader_get_num_files(&zipArchive);
		for (int i = 0; i < numFiles; i++) {
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&zipArchive, i, &file_stat))
			{
				printf("mz_zip_reader_file_stat() failed!\n");
				mz_zip_reader_end(&zipArchive);
				return EXIT_FAILURE;
			}

			size_t maxFilenameSize = 512;
			char* filename = (char*)malloc(maxFilenameSize);
			// returns length of filename + null char
			unsigned int filepathLength = mz_zip_reader_get_filename(&zipArchive, i, filename, maxFilenameSize);
			mz_bool isDir = mz_zip_reader_is_file_a_directory(&zipArchive, i);
			//printf("%s\nisDir: %d\n", filename, isDir);

			// -1 because we dont want null terminator
			std::string filepath(filename, filepathLength - 1);
			//printf("last char of filepath: %d\n", filepath[filepath.size() - 1]);

			if (isDir) {
				fs::create_directories("./include/dependencies/" + filepath);
			}
			else {
				std::string dstFilepath = "./include/dependencies/";
				dstFilepath += filename;

				// should receive same flags as mz_zip_reader_init_file()
				status = mz_zip_reader_extract_file_to_file(&zipArchive, filename, dstFilepath.c_str(), 0);
				if (!status) {
					printf("mz_zip_reader_extract_file_to_file() failed!\n");
					mz_zip_reader_end(&zipArchive);
					return EXIT_FAILURE;
				}
			}
		}

		mz_zip_reader_end(&zipArchive);
		return 0;
	}
}