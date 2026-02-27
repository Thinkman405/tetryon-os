#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tetryon_fs.h"

#define MAX_FILES 1024
#define STORAGE_DIR "storage"

// Helper to check/create storage directory
void ensure_storage_dir() {
    struct stat st = {0};
    if (stat(STORAGE_DIR, &st) == -1) {
        #ifdef _WIN32
            mkdir(STORAGE_DIR);
        #else
            mkdir(STORAGE_DIR, 0700);
        #endif
    }
}

void fs_init() {
    printf("[GeoFS] Initializing Crystalline Storage (Production Mode)...\n");
    ensure_storage_dir();
}

// Crystallize: Persist data into a rigid geometric structure on disk
uint64_t crystallize(uint64_t file_id, TetryonNode* data, int count) {
    char filepath[64];
    sprintf(filepath, "%s/crystal_%llu.geo", STORAGE_DIR, file_id);

    printf("[GeoFS] Crystallizing data to %s...\n", filepath);

    FILE* f = fopen(filepath, "wb");
    if (!f) {
        printf("[GeoFS] Error: Could not open crystal file for writing.\n");
        return 0;
    }

    // Write Header
    CrystalHeader header;
    header.file_id = file_id;
    header.size = count;
    header.start_node_sig = 0; // Not used in disk format, but kept for struct alignment
    fwrite(&header, sizeof(CrystalHeader), 1, f);

    // Write Nodes (Payloads only for this simple implementation)
    // In a full graph, we'd serialize the links too, but here it's a linear chain.
    for (int i = 0; i < count; i++) {
        fwrite(&data[i], sizeof(TetryonNode), 1, f);
    }

    fclose(f);
    printf("[GeoFS] Crystallization complete. Size: %d nodes.\n", count);
    return 1; // Success
}

// Sublimate: Read a Crystal back into active Spiral Memory from disk
int sublimate(uint64_t file_id, TetryonNode* buffer, int max_count) {
    char filepath[64];
    sprintf(filepath, "%s/crystal_%llu.geo", STORAGE_DIR, file_id);

    FILE* f = fopen(filepath, "rb");
    if (!f) {
        printf("[GeoFS] Error: Crystal file %s not found.\n", filepath);
        return 0;
    }

    printf("[GeoFS] Sublimating from %s...\n", filepath);

    // Read Header
    CrystalHeader header;
    if (fread(&header, sizeof(CrystalHeader), 1, f) != 1) {
        printf("[GeoFS] Error: Corrupt crystal header.\n");
        fclose(f);
        return 0;
    }

    int count = 0;
    while (count < max_count && count < header.size) {
        if (fread(&buffer[count], sizeof(TetryonNode), 1, f) != 1) break;
        count++;
    }

    fclose(f);
    printf("[GeoFS] Sublimation complete. Loaded %d nodes.\n", count);
    return count;
}
