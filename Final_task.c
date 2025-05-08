#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_REPORTS 100

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

typedef struct {
    char area[50];
    char incident_type[50];
    char date[11];
} incident;

incident reports[MAX_REPORTS];
int count_report = 0;

void clear()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void low_case(char str[])
{
    for (int i = 0; str[i]; i++)
        str[i] = tolower(str[i]);
}

void get_valid_input(char *prompt, char *input, int max_len)
{
    do
    {
        printf("%s", prompt);
        fgets(input, max_len, stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
            printf(RED "🚫 Input cannot be empty! Please try again.\n" RESET);

    } while (strlen(input) == 0);
}

int validate_date(const char *date)
{
    if (strlen(date) != 10) return 0;
    if (date[2] != '.' || date[5] != '.') return 0;

    int day, month, year;
    if (sscanf(date, "%2d.%2d.%4d", &day, &month, &year) != 3) return 0;
    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1900) return 0;

    return 1;
}

void get_valid_date(char *prompt, char *input)
{
    do
    {
        printf("%s", prompt);
        fgets(input, 11, stdin);
        input[strcspn(input, "\n")] = '\0';

        if (!validate_date(input))
            printf(RED "🚫 Invalid date format! Please use dd.mm.yyyy.\n" RESET);

    } while (!validate_date(input));
}

void save_all_incidents_to_file()
{
    FILE *file = fopen("incidents.txt", "w");
    if (!file)
    {
        printf(RED "Error saving file.\n" RESET);
        return;
    }

    for (int i = 0; i < count_report; i++)
        fprintf(file, "%s,%s,%s\n", reports[i].area, reports[i].incident_type, reports[i].date);

    fclose(file);
}

void load_incidents_from_file()
{
    FILE *file = fopen("incidents.txt", "r");
    if (!file) return;

    char line[200];
    while (fgets(line, sizeof(line), file) && count_report < MAX_REPORTS)
    {
        char *token = strtok(line, ",");
        if (token)
        {
            strcpy(reports[count_report].area, token);
            token = strtok(NULL, ",");
            if (token)
            {
                strcpy(reports[count_report].incident_type, token);
                token = strtok(NULL, "\n");
                if (token)
                {
                    strcpy(reports[count_report].date, token);
                    count_report++;
                }
            }
        }
    }

    fclose(file);
}

int compare_by_area(const void *a, const void *b)
{
    return strcmp(((incident *)a)->area, ((incident *)b)->area);
}

int compare_by_date(const void *a, const void *b)
{
    incident *inc1 = (incident *)a;
    incident *inc2 = (incident *)b;

    int d1, m1, y1;
    int d2, m2, y2;

    sscanf(inc1->date, "%2d.%2d.%4d", &d1, &m1, &y1);
    sscanf(inc2->date, "%2d.%2d.%4d", &d2, &m2, &y2);

    if (y1 != y2)
        return y1 - y2;
    if (m1 != m2)
        return m1 - m2;
    return d1 - d2;
}

void view_incidents(int pause)
{
    clear();
    if (count_report == 0)
    {
        printf(YELLOW "No incidents reported yet.\n" RESET);
    }
    else
    {
        printf("=== Incident Reports ===\n\n");
        printf("%-4s %-20s %-20s %-12s\n", "#", "Area", "Incident Type", "Date");
        printf("---------------------------------------------------------\n");

        for (int i = 0; i < count_report; i++)
            printf("%-4d %s%-20s%s %s%-20s%s %s%-12s%s\n", i + 1,
                   BLUE, reports[i].area, RESET,
                   YELLOW, reports[i].incident_type, RESET,
                   GREEN, reports[i].date, RESET);
    }

    if (pause)
    {
        printf("\nPress Enter to return...");
        getchar();
    }
}

void add_incident()
{
    clear();
    printf("=== Add Incident Report ===\n");
    if (count_report >= MAX_REPORTS)
    {
        printf(RED "Storage full.\n" RESET);
        return;
    }

    incident new_incident;

    get_valid_input("✅ Enter area: ", new_incident.area, 50);
    get_valid_input("✅ Enter incident type: ", new_incident.incident_type, 50);
    get_valid_date("✅ Enter date of incident (dd.mm.yyyy): ", new_incident.date);

    reports[count_report++] = new_incident;
    save_all_incidents_to_file();

    printf(GREEN "✅ Incident added successfully!\n" RESET);
    getchar();
}

void filter_incidents()
{
    clear();
    int filterChoice;
    char keyword[50], temp[50];

    printf("=== Filter Incident Reports ===\n");
    printf("1. Filter by Area\n");
    printf("2. Filter by Incident Type\n");
    printf("3. Filter by Date\n");
    printf("4. Back to Menu\n");
    printf("Choose an option: ");
    scanf("%d", &filterChoice);
    getchar();

    if (filterChoice == 4) return;

    get_valid_input("Enter keyword: ", keyword, 50);
    low_case(keyword);

    clear();
    printf("=== Filtered Results ===\n");
    int found = 0;

    for (int i = 0; i < count_report; i++)
    {
        if (filterChoice == 1)
            strcpy(temp, reports[i].area);
        else if (filterChoice == 2)
            strcpy(temp, reports[i].incident_type);
        else
            strcpy(temp, reports[i].date);

        low_case(temp);

        if (strstr(temp, keyword))
        {
            printf("%-4d %s%-20s%s %s%-20s%s %s%-12s%s\n",
                   i + 1, BLUE, reports[i].area, RESET, YELLOW, reports[i].incident_type, RESET, GREEN, reports[i].date, RESET);
            found = 1;
        }
    }

    if (!found)
        printf(RED "No matching incidents found.\n" RESET);

    printf("\nPress Enter to return...");
    getchar();
}

void edit_incident()
{
    clear();
    if (count_report == 0)
    {
        printf(YELLOW "No incidents to edit.\n" RESET);
        getchar();
        return;
    }

    view_incidents(0);
    int index;
    printf("\nEnter incident number to edit: ");
    scanf("%d", &index);
    getchar();

    if (index < 1 || index > count_report)
    {
        printf(RED "Invalid incident number.\n" RESET);
        getchar();
        return;
    }

    incident *inc = &reports[index - 1];

    get_valid_input("✅ Enter new area: ", inc->area, 50);
    get_valid_input("✅ Enter new incident type: ", inc->incident_type, 50);
    get_valid_date("✅ Enter new date (dd.mm.yyyy): ", inc->date);

    save_all_incidents_to_file();
    printf(GREEN "✅ Incident updated successfully!\n" RESET);
    getchar();
}

void delete_incident()
{
    clear();
    if (count_report == 0)
    {
        printf(YELLOW "No incidents to delete.\n" RESET);
        getchar();
        return;
    }

    view_incidents(0);
    int index;
    printf("\nEnter incident number to delete: ");
    scanf("%d", &index);
    getchar();

    if (index < 1 || index > count_report)
    {
        printf(RED "Invalid incident number.\n" RESET);
        getchar();
        return;
    }

    for (int i = index - 1; i < count_report - 1; i++)
        reports[i] = reports[i + 1];
    count_report--;

    save_all_incidents_to_file();
    printf(GREEN "✅ Incident deleted successfully!\n" RESET);
    getchar();
}

void sort_incidents()
{
    clear();
    if (count_report == 0)
    {
        printf(YELLOW "No incidents to sort.\n" RESET);
        getchar();
        return;
    }

    printf("=== Sort Incidents ===\n");
    printf("1. Sort by Area\n");
    printf("2. Sort by Date\n");
    printf("3. Back to Menu\n");
    printf("Choose an option: ");
    int choice;
    scanf("%d", &choice);
    getchar();

    if (choice == 1)
        qsort(reports, count_report, sizeof(incident), compare_by_area);
    else if (choice == 2)
        qsort(reports, count_report, sizeof(incident), compare_by_date);
    else
        return;

    save_all_incidents_to_file();
    printf(GREEN "✅ Incidents sorted.\n" RESET);
    getchar();
}

void display_menu()
{
    int choice;
    char line[10];

    while (1)
    {
        clear();
        printf("=== " GREEN "Incident Reporting System" RESET " ===\n");
        printf("1. Add Incident Report\n");
        printf("2. View Incident Reports\n");
        printf("3. Filter Incidents\n");
        printf("4. Edit Incident\n");
        printf("5. Delete Incident\n");
        printf("6. Sort Incidents\n");
        printf("7. Exit\n");
        printf("Choose an option: ");

        fgets(line, sizeof(line), stdin);
        choice = atoi(line);

        switch (choice)
        {
        case 1: add_incident(); break;
        case 2: view_incidents(1); break;
        case 3: filter_incidents(); break;
        case 4: edit_incident(); break;
        case 5: delete_incident(); break;
        case 6: sort_incidents(); break;
        case 7:
            clear();
            printf(GREEN "Exiting... Goodbye!\n" RESET);
            return;
        default:
            printf(RED "Invalid choice!\n" RESET);
            getchar();
        }
    }
}

int main()
{
    load_incidents_from_file();
    display_menu();
    return 0;
}