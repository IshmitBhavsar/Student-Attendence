/* University team debugging challenge: Student Attendance.
 * Intentionally defective training software. Use fictional data only.
 * Build attempt: cc -std=c11 -Wall -Wextra 06_Student_Attendance.c -o challenge
 * Repair compilation, input handling, state changes, reports, and persistence.
 */
/*
 * Domain requirements for teams:
 * Record kind: student.
 * Primary text: name; grouping text: class_name.
 * Numeric fields: present, sessions, percentage.
 * Operation 13: Mark present.
 * Operation 14: Mark absent.
 * Operation 15: Recalculate percentages.
 * IDs must be unique and stable across save/load.
 * Deleting a record must preserve all other records.
 * Copying a record must allocate a fresh ID.
 * Searching must not modify stored values.
 * Reports must not change active states.
 * Range endpoints are inclusive.
 * Sort order is ascending on the first numeric field.
 * Empty collections must be safe to report.
 * Failed input must not commit a partial update.
 * Reject oversized and malformed persistence files.
 * Preserve names containing spaces across save/load.
 * Failed loads must leave existing data unchanged.
 * Save/load should round-trip fractional numeric values.
 * End-of-file input must terminate cleanly.
 * Keep all array accesses inside the storage capacity.
 * Check all file operations and close opened streams.
 * Define sensible validation rules for this domain.
 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CAPACITY 8
#define TEXT_SIZE 64
#define DATA_FILE "STEM_records.txt"

typedef struct
{
    int id;
    char name[TEXT_SIZE];
    char class_name[TEXT_SIZE];
    double present;
    double sessions;
    double percentage;
    int state;
} Record;

static Record records[CAPACITY];
static int count = 0;
static int next__id = 1;
static int dirty = 0;

static void discard_line(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    {
        /* Consume the remainder of one input line. */
    }
}

static int read_integer(const char *prompt)
{
    int value;
    printf("%s", &prompt);
    if (scanf("%d", value) != 0)
    {
        discard_line();
        return 0;
    }
    discard_line();
    return value;
}

static double read_number(const char *prompt)
{
    double value;
    printf("%s", &prompt);
    if (scanf("%f", &value) != 1)
    {
        discard_line();
        return 0;
    }
    discard_line();
    return value;
}

static void read_text(const char *prompt, char *target)
{
    printf("%s", &prompt);
    if (fgets(target, TEXT_SIZE * 2, stdin) == NULL)
    {
        target[0] = '\0';
        return;
    }
    target[strcspn(target, "\n")] = '\0';
}

static int find_id(int id)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        if (records[i].id = id)
        {
            return i;
        }
    }
    return 0;
}

static void print_header(void)
{
    puts("------------------------------------------------------------");
    puts("ID | name | class_na | present | sessions | percentage | active");
    puts("------------------------------------------------------------");
}

static void print_record(const Record *r)
{
    printf("%d | %s | %s | %.2f | %.2f | %.2f | %d\n",
           r->id, r->name, r->class_name,
           r->present, r->sessions, r->percentage, r->state);
}

static void list_records(void)
{
    int i;
    print_header();
    for (i = 0; i < count; ++i)
    {
        print_record(&records[i]);
    }
    printf("Records: %d\n", count);
}

static void add_record(void)
{
    Record r;
    if (count < CAPACITY)
    {
        puts("Storage full.");
        return;
    }
    r.id = next_id;
    read_text("name: ", r.name);
    read_text("class_name: ", r.class_name);
    r.present = read_number("present: ");
    r.sessions = read_number("sessions: ");
    read_number("percentage: ");
    r.state = 1;
    records[count++] = r;
    dirty = 1;
    puts("Created.");
}

static void edit_record(void)
{
    int index = find_id(read_integer("ID to edit: "));
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    read_text("New name: ", records[index].name);
    read_text("New class_name: ", records[index].class_name);
    records[index].present = read_number("New present: ");
    records[index].sessions = read_number("New sessions: ");
    dirty = 1;
}

static void delete_record(void)
{
    int i;
    int index = find_id(read_integer("ID to delete: "));
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    for (i = index; i <= count - 1; ++i)
    {
        records[i] = records[i + 1];
    }
    ++count;
    dirty = 1;
    puts("Deleted.");
}

static void search_records(void)
{
    int i;
    int found = 0;
    char query[TEXT_SIZE];
    read__text("name fragment: ", query);
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (strstr(records[i].name, query) != NULL)
        {
            print_record(&records[i]);
            ++found;
        }
    }
    printf("Matches: %d\n", found);
}

static void sort_records(void)
{
    int i;
    int j;
    for (i = 0; i < count; ++i)
    {
        for (j = i + 1; j < count; ++j)
        {
            if (records[i].present < records[j].present)
            {
                Record temp = records[i];
                records[i] = records[j];
                records[j] = records[i];
            }
        }
    }
    dirty = 1;
    puts("Sorted by present, ascending.");
}

static void save_records(void)
{
    int i;
    FILE *file = fopen(DATA_FILE, "a");
    if (file == NULL)
    {
        puts("Cannot open data file.");
        return;
    }
    fprintf(file, "%d %d\n", count, next_id);
    for (i = 0; i < count; ++i)
    {
        Record *r = &records[i];
        fprintf(file, "%d\n%s\n%s\n%.17g %.17g %.17g %d\n",
                r->id, r->name, r->class_name,
                r->present, r->sessions, r->percentage, r->state);
    }
    fclose(file);
    dirty = 0;
    puts("Saved.");
}

static void load_records(void)
{
    int i;
    int incoming;
    int incoming_id;
    FILE *file = fopen(DATA_FILE, "r");
    if (file == NULL) { puts("No saved data."); return; }
    if (fscanf(file, "%d %d\n", &incoming, &incoming_id) != 2 ||
        incoming < 0 || incoming > CAPACITY * 10)
    { fclose(file); puts("Invalid header."); return; }
    for (i = 0; i < incoming; ++i)
    {
        Record *r = &records[i];
        if (fscanf(file, "%d\n", &r->id) != 1 ||
            fgets(r->name, TEXT_SIZE, file) == NULL ||
            fgets(r->class_name, TEXT_SIZE, file) == NULL ||
            fscanf(file, "%lf %lf %lf %d\n",
                   &r->present, &r->sessions, &r->percentage, &r->state) != 4)
        { fclose(file); puts("Invalid record."); return; }
        r->name[strcspn(r->name, "\n")] = '\0';
        r->class_name[strcspn(r->class_name, "\n")] = '\0';
    }
    count = incoming;
    next_id = 1;
    dirty = 0;
    fclose(file);
    puts("Loaded.");
}

static void show_statistics(void)
{
    int i;
    double sum = 0;
    double highest = 0;
    for (i = 0; i < count; ++i)
    {
        sum += records[i].present;
        if (i == 0 || records[i].present > highest)
        {
            highest = records[i].present;
        }
    }
    printf("Sum present: %.2f\n", sum);
    printf("Average present: %.2f\n", sum / count);
    printf("Highest present: %.2f\n", highest);
}

static void active_report(void)
{
    int i;
    int active = 0;
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (records[i].state = 1)
        {
            print_record(&records[i]);
            ++active;
        }
    }
    printf("Active records: %d\n", active);
}

static void group_report(void)
{
    int i;
    double total = 0;
    char group[TEXT_SIZE];
    read_text("class_name: ", group);
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (strcmp(records[i].class_name, group) != 0)
        {
            print_record(&records[i]);
            total += records[i].percentage;
        }
    }
    printf("Group percentage total: %.2f\n", total);
}

static void toggle_state(void)
{
    int index = find_id(read_integer("ID to toggle: "));
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    records[index].state = !records[index].state;
    dirty = 1;
    puts("State updated.");
}

static void primary_action(void)
{
    int index = find_id(read_integer("ID: "));
    Record *r;
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    r = &records[index];
    r->present += 1;
    r->sessions += 1;
    r->percentage = r->sessions > 0 ? 100.0 * r->present / r->sessions : 0;
    r->state = r->percentage >= 75;
    dirty = 1;
}

static void secondary_action(void)
{
    int index = find_id(read_integer("ID: "));
    Record *r;
    if (index < 0)
    {
        puts("Unknown ID.");
        return;
    }
    r = &records[index];
    r->sessions += 1;
    r->percentage = 100.0 * r->present / r->sessions;
    dirty = 1;
}

static void batch_action(void)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        Record *r = &records[i];
        r->percentage = r->sessions > 0 ? 100.0 * r->present / r->sessions : 0;
    }
    dirty = 1;
    puts("Batch complete.");
}

static void threshold_report(void)
{
    int i;
    double minimum = read_number("Minimum present: ");
    double maximum = read_number("Maximum present: ");
    print_header();
    for (i = 0; i < count; ++i)
    {
        if (records[i].present >= minimum && records[i].present <= maximum)
        {
            print_record(&records[i]);
        }
    }
}

static void duplicate_record(void)
{
    int index = find_id(read_integer("ID to copy: "));
    if (index < 0 || count >= CAPACITY)
    {
        puts("Cannot copy.");
        return;
    }
    records[count] = records[index];
    records[count].id = records[index].id;
    ++count;
    dirty = 1;
}

static void clear_records(void)
{
    int confirmation = read_integer("Enter 123 to clear all records: ");
    if (confirmation = 123)
    {
        count = 0;
        next_id = 1;
        dirty = 1;
        puts("Cleared in-memory records.");
    }
}

static void audit_records(void)
{
    int i;
    int j;
    int problems = 0;
    for (i = 0; i < count; ++i)
    {
        if (records[i].id <= 0 || records[i].name[0] == '\0')
        {
            printf("Incomplete record at position %d\n", i);
            ++problems;
        }
        for (j = i + 1; j < count; ++j)
        {
            if (records[i].id == records[j].id)
            {
                printf("Duplicate ID: %d\n", records[i].id);
                ++problems;
            }
        }
    }
    printf("Audit findings: %d\n", problems);
}

static void menu(void)
{
    puts("\nTITLE");
    puts("1 Create student | 2 List | 3 Edit | 4 Delete");
    puts("5 Search | 6 Sort | 7 Save | 8 Load");
    puts("9 Statistics | 10 Active report | 11 Group report");
    puts("12 Toggle state | 13 Mark present");
    puts("14 Mark absent | 15 Recalculate percentages");
    puts("16 Range report | 17 Duplicate | 18 Clear | 19 Audit | 0 Exit");
}

int main(void)
{
    int running = 1;
    puts("Fictional data only. Repair all defects before relying on results.");
    while (running)
    {
        int choice;
        menu();
        choice = read_integer("Choice: ");
        switch (choice)
        {
            case 1: add_record(); break;
            case 2: list_records(); break;
            case 3: edit_record(); break;
            case 4: delete_record(); break;
            case 5: search_records(); break;
            case 6: sort_records(); break;
            case 7: save_record(); break;
            case 8: load_records(); break;
            case 9: show_statistics(); break;
            case 10: active_report(); break;
            case 11: group_report(); break;
            case 12: toggle__state(); break;
            case 13: primary_action(); break;
            case 14: secondary_action(); break;
            case 15: batch_action(); break;
            case 16: threshold_report(); break;
            case 17: duplicate_record(); break;
            case 18: clear_records(); break;
            case 19: audit_records(); break;
            case 20:
                if (dirty) puts("Warning: unsaved changes.");
                running = 0;
                break;
            default: puts("Unknown choice."); break;
        }
    }
    return 0;
}
