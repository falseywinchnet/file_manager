#include <sqlite3.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static void fail(sqlite3 *db, const char *operation, int code) {
    fprintf(stderr, "%s: sqlite=%d message=%s\n", operation, code, db ? sqlite3_errmsg(db) : "none");
    exit(1);
}

static void execute(sqlite3 *db, const char *sql) {
    char *message = NULL;
    int code = sqlite3_exec(db, sql, NULL, NULL, &message);
    if (code != SQLITE_OK) {
        fprintf(stderr, "sql failed: %s: %s\n", sql, message ? message : sqlite3_errmsg(db));
        sqlite3_free(message);
        exit(1);
    }
}

static uint64_t monotonic_ns(void) {
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) {
        perror("clock_gettime");
        exit(1);
    }
    return (uint64_t)value.tv_sec * UINT64_C(1000000000) + (uint64_t)value.tv_nsec;
}

static int compare_u64(const void *left, const void *right) {
    uint64_t a = *(const uint64_t *)left;
    uint64_t b = *(const uint64_t *)right;
    return (a > b) - (a < b);
}

static uint64_t percentile(const uint64_t *values, size_t count, size_t numerator) {
    size_t index = (count * numerator + 99) / 100;
    if (index == 0) {
        return values[0];
    }
    return values[index - 1];
}

static void bind_i64(sqlite3 *db, sqlite3_stmt *statement, int index, int64_t value) {
    int code = sqlite3_bind_int64(statement, index, value);
    if (code != SQLITE_OK) fail(db, "bind integer", code);
}

static void bind_text(sqlite3 *db, sqlite3_stmt *statement, int index, const char *value) {
    int code = sqlite3_bind_text(statement, index, value, -1, SQLITE_TRANSIENT);
    if (code != SQLITE_OK) fail(db, "bind text", code);
}

static void run_insert(sqlite3 *db, sqlite3_stmt *statement) {
    int code = sqlite3_step(statement);
    if (code != SQLITE_DONE) fail(db, "insert", code);
    code = sqlite3_reset(statement);
    if (code != SQLITE_OK) fail(db, "reset insert", code);
    sqlite3_clear_bindings(statement);
}

static sqlite3_stmt *prepare(sqlite3 *db, const char *sql) {
    sqlite3_stmt *statement = NULL;
    int code = sqlite3_prepare_v2(db, sql, -1, &statement, NULL);
    if (code != SQLITE_OK) fail(db, "prepare", code);
    return statement;
}

static void sample_query(sqlite3 *db, sqlite3_stmt *statement, const char *value,
                         size_t iterations, int expected_rows, uint64_t expected_sum,
                         const char *label) {
    uint64_t *samples = calloc(iterations, sizeof(*samples));
    if (!samples) {
        perror("calloc samples");
        exit(1);
    }
    uint64_t aggregate = 0;
    for (size_t iteration = 0; iteration < iterations; iteration++) {
        bind_text(db, statement, 1, value);
        uint64_t started = monotonic_ns();
        int rows = 0;
        uint64_t sum = 0;
        uint64_t touched = 0;
        int code;
        while ((code = sqlite3_step(statement)) == SQLITE_ROW) {
            sum += (uint64_t)sqlite3_column_int64(statement, 0);
            const unsigned char *path = sqlite3_column_text(statement, 1);
            const unsigned char *name = sqlite3_column_text(statement, 2);
            const unsigned char *kind = sqlite3_column_text(statement, 3);
            touched += (uint64_t)sqlite3_column_bytes(statement, 1);
            touched += (uint64_t)sqlite3_column_bytes(statement, 2);
            touched += (uint64_t)sqlite3_column_bytes(statement, 3);
            touched += (uint64_t)sqlite3_column_int64(statement, 4);
            touched += (uint64_t)sqlite3_column_int64(statement, 5);
            touched += (uint64_t)sqlite3_column_int64(statement, 6);
            if (!path || !name || !kind) {
                fprintf(stderr, "%s returned NULL exact text\n", label);
                exit(1);
            }
            rows++;
        }
        samples[iteration] = monotonic_ns() - started;
        if (code != SQLITE_DONE) fail(db, "query", code);
        if (rows != expected_rows || sum != expected_sum) {
            fprintf(stderr, "%s correctness rows=%d sum=%" PRIu64 " expected_rows=%d expected_sum=%" PRIu64 "\n",
                    label, rows, sum, expected_rows, expected_sum);
            exit(1);
        }
        aggregate ^= sum + (uint64_t)rows + touched;
        code = sqlite3_reset(statement);
        if (code != SQLITE_OK) fail(db, "reset query", code);
        sqlite3_clear_bindings(statement);
    }
    qsort(samples, iterations, sizeof(*samples), compare_u64);
    printf("distribution=%s samples=%zu p50_ns=%" PRIu64 " p95_ns=%" PRIu64
           " p99_ns=%" PRIu64 " max_ns=%" PRIu64 " correctness_xor=%" PRIu64 "\n",
           label, iterations, percentile(samples, iterations, 50), percentile(samples, iterations, 95),
           percentile(samples, iterations, 99), samples[iterations - 1], aggregate);
    free(samples);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s DATABASE FILE_COUNT QUERY_ITERATIONS\n", argv[0]);
        return 2;
    }
    const char *database_path = argv[1];
    char *end = NULL;
    long file_count_long = strtol(argv[2], &end, 10);
    if (!end || *end || file_count_long < 10000) {
        fprintf(stderr, "FILE_COUNT must be at least 10000\n");
        return 2;
    }
    long iterations_long = strtol(argv[3], &end, 10);
    if (!end || *end || iterations_long < 1) {
        fprintf(stderr, "QUERY_ITERATIONS must be positive\n");
        return 2;
    }
    int64_t file_count = (int64_t)file_count_long;
    size_t iterations = (size_t)iterations_long;
    const int64_t directory_size = 10;
    int64_t directory_count = (file_count + directory_size - 1) / directory_size;

    sqlite3 *db = NULL;
    int code = sqlite3_open_v2(database_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (code != SQLITE_OK) fail(db, "open", code);
    sqlite3_busy_timeout(db, 10000);
    execute(db, "PRAGMA journal_mode=DELETE; PRAGMA synchronous=FULL; PRAGMA temp_store=MEMORY; PRAGMA cache_size=-8192; PRAGMA mmap_size=0; PRAGMA foreign_keys=ON;");

    uint64_t build_started = monotonic_ns();
    execute(db,
        "CREATE TABLE objects(object_id INTEGER PRIMARY KEY, kind INTEGER NOT NULL, size INTEGER NOT NULL, mode INTEGER NOT NULL, modified_ns INTEGER NOT NULL);"
        "CREATE TABLE bindings(path TEXT PRIMARY KEY, name TEXT NOT NULL, object_id INTEGER NOT NULL REFERENCES objects(object_id), parent_id INTEGER NOT NULL REFERENCES objects(object_id)) WITHOUT ROWID;"
        "BEGIN IMMEDIATE;");
    sqlite3_stmt *insert_object = prepare(db, "INSERT INTO objects(object_id,kind,size,mode,modified_ns) VALUES(?,?,?,?,?)");
    sqlite3_stmt *insert_binding = prepare(db, "INSERT INTO bindings(path,name,object_id,parent_id) VALUES(?,?,?,?)");

    bind_i64(db, insert_object, 1, 0);
    bind_i64(db, insert_object, 2, 2);
    bind_i64(db, insert_object, 3, 0);
    bind_i64(db, insert_object, 4, 0);
    bind_i64(db, insert_object, 5, 0);
    run_insert(db, insert_object);
    for (int64_t directory = 0; directory < directory_count; directory++) {
        char name[64];
        snprintf(name, sizeof(name), "dir-%07" PRId64, directory);
        int64_t object_id = file_count + directory + 1;
        bind_i64(db, insert_object, 1, object_id);
        bind_i64(db, insert_object, 2, 2);
        bind_i64(db, insert_object, 3, 0);
        bind_i64(db, insert_object, 4, 0);
        bind_i64(db, insert_object, 5, 0);
        run_insert(db, insert_object);
        bind_text(db, insert_binding, 1, name);
        bind_text(db, insert_binding, 2, name);
        bind_i64(db, insert_binding, 3, object_id);
        bind_i64(db, insert_binding, 4, 0);
        run_insert(db, insert_binding);
    }
    for (int64_t index = 0; index < file_count; index++) {
        char name[64];
        char path[128];
        int64_t directory = index / directory_size;
        if (index % directory_size == 0) {
            strcpy(name, "repeated");
        } else {
            snprintf(name, sizeof(name), "file-%09" PRId64, index);
        }
        snprintf(path, sizeof(path), "dir-%07" PRId64 "/%s", directory, name);
        int64_t object_id = index + 1;
        int64_t parent_id = file_count + directory + 1;
        bind_i64(db, insert_object, 1, object_id);
        bind_i64(db, insert_object, 2, 1);
        bind_i64(db, insert_object, 3, index);
        bind_i64(db, insert_object, 4, 0);
        bind_i64(db, insert_object, 5, index * 1000);
        run_insert(db, insert_object);
        bind_text(db, insert_binding, 1, path);
        bind_text(db, insert_binding, 2, name);
        bind_i64(db, insert_binding, 3, object_id);
        bind_i64(db, insert_binding, 4, parent_id);
        run_insert(db, insert_binding);
    }
    sqlite3_finalize(insert_object);
    sqlite3_finalize(insert_binding);
    execute(db, "COMMIT; CREATE INDEX bindings_name_path ON bindings(name,path); PRAGMA optimize;");
    uint64_t build_ns = monotonic_ns() - build_started;

    struct stat database_stat;
    if (stat(database_path, &database_stat) != 0) {
        perror("stat database");
        return 1;
    }
    int64_t binding_count = file_count + directory_count;
    printf("sqlite=%s file_objects=%" PRId64 " bindings=%" PRId64 " build_ns=%" PRIu64
           " bytes=%lld bytes_per_binding=%.2f\n",
           sqlite3_libversion(), file_count, binding_count, build_ns, (long long)database_stat.st_size,
           (double)database_stat.st_size / (double)binding_count);

    int64_t unique_index = file_count / 2 + 1;
    char unique_name[64];
    char unique_path[128];
    snprintf(unique_name, sizeof(unique_name), "file-%09" PRId64, unique_index);
    snprintf(unique_path, sizeof(unique_path), "dir-%07" PRId64 "/%s", unique_index / directory_size, unique_name);
    sqlite3_stmt *path_query = prepare(db,
        "SELECT o.object_id,b.path,b.name,o.kind,o.size,o.mode,o.modified_ns FROM bindings AS b JOIN objects AS o ON o.object_id=b.object_id WHERE b.path=?1");
    sqlite3_stmt *name_query = prepare(db,
        "SELECT o.object_id,b.path,b.name,o.kind,o.size,o.mode,o.modified_ns FROM bindings AS b JOIN objects AS o ON o.object_id=b.object_id WHERE b.name=?1 ORDER BY b.path LIMIT 100");
    sample_query(db, path_query, unique_path, iterations, 1, (uint64_t)(unique_index + 1), "sqlite-exact-path");
    sample_query(db, name_query, "repeated", iterations, 100, 49600, "sqlite-name-first-100");
    sqlite3_finalize(path_query);
    sqlite3_finalize(name_query);
    sqlite3_close(db);
    return 0;
}
