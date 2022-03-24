#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE "cat /sys/class/Sysfs_class/sysfs_class_sysfs_Device/sysfs_att"
#define LINE2 "sudo bash -c "
#define REST "echo '0' > /sys/class/Sysfs_class/sysfs_class_sysfs_Device/sysfs_att"

char buf[255];
// excutes ./a.out
void process_command(char *line)
{
    sprintf(buf, "%s", line);
    system(buf);
}
// excutes ./a.out 0
void chain_commands()
{

    int i, j;
    for (i = 0; i < strlen(LINE2); i++)
    {
        buf[i] = LINE2[i];
    }
    buf[i] = '\'';
    i++;
    for (j = 0; j < strlen(REST); j++)
    {
        buf[i + j] = REST[j];
    }
    buf[i + j] = '\'';
    j++;

    system(buf);
}

int main(int argc, char **argv)
{

    int counter = 0;

    if (argc == 1)
    {
        process_command(LINE);
    }
    else if (argc == 2 && strcmp(argv[1], "0") == 0)
    {
        chain_commands();
    }
    else
    {
        fprintf(stderr, "error in arguments");
        exit(1);
    }
    return 0;
}
