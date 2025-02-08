#include "builtins/builtins.h"

static void print_E(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        fputs(args[i], stdout);
        if (args[i][0] != '\0' && args[i + 1] != NULL)
        {
            putchar(' ');
        }
        i++;
    }
}

static void print_e(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        char *str = args[i];
        while (*str)
        {
            if (*str == '\\' && *(str + 1))
            {
                str++;
                switch (*str)
                {
                case 'n':
                    putchar('\n');
                    break;
                case 't':
                    putchar('\t');
                    break;
                case '\\':
                    putchar('\\');
                    break;
                default:
                    putchar('\\');
                    putchar(*str);
                }
            }
            else
            {
                putchar(*str);
            }
            str++;
        }
        if (args[i + 1] != NULL)
        {
            putchar(' ');
        }
        i++;
    }
}

int echo(struct ast_builtin *command)
{
    int i = 0;
    int nflag = 0;
    int eflag = 0;
    int cond = 1;
    while (cond && command->args[i] && command->args[i][0] == '-')
    {
        int j = 1;
        while (cond && command->args[i][j])
        {
            switch (command->args[i][j])
            {
            case 'n':
                nflag = 1;
                break;
            case 'e':
                eflag = 1;
                break;
            case 'E':
                eflag = 0;
                break;
            default:
                cond = 0;
                i--;
                break;
            }
            j++;
        }
        i++;
    }
    if (eflag)
    {
        print_e(command->args + i);
    }
    else
    {
        print_E(command->args + i);
    }
    if (!nflag)
    {
        putchar('\n');
    }
    fflush(stdout);
    return 0;
}
