#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "archivator_logic/archivator_api.h"

typedef enum
{
    optionType,
    argType
} ETokenTypes;

typedef struct
{
    ETokenTypes type;
    char optionValue;
    char *argValue;
} TToken;

typedef struct
{
    TToken *tokens;
    int length;
    int capacity;
} TTokenArr;

TSetupSettings *get_settings()
{
    TSetupSettings *settings = (TSetupSettings *)malloc(sizeof(TSetupSettings));
    settings->mode = null;
    settings->withInfo = false;

    settings->dirToArchivate = NULL;
    settings->filesCount = 0;
    settings->filesToArchivate = NULL;

    settings->archivePath = NULL;
    settings->destDir = NULL;

    settings->isError = false;
    settings->errorMessage = (char *)malloc(ERROR_LENGTH * sizeof(char));

    return settings;
}

void delete_settings(TSetupSettings *settings)
{
    free(settings->errorMessage);
    free(settings);
}

static void append_token(TTokenArr *arr, TToken value)
{
    if (arr->length == arr->capacity)
    {
        arr->capacity *= 2;
        arr->tokens = (TToken *)realloc(arr->tokens, arr->capacity * sizeof(TToken));
    }
    arr->tokens[arr->length] = value;
    arr->length++;
}

static bool is_option(char *value)
{
    if (strlen(value) <= 1)
    {
        return false;
    }
    return value[0] == '-';
}

static TToken get_option_token(char optValue)
{
    TToken token;
    token.type = optionType;
    token.optionValue = optValue;
    return token;
}

static TToken get_arg_token(char *argValue)
{
    TToken token;
    token.type = argType;
    token.argValue = argValue;
    return token;
}

static TTokenArr *read_tokens(int argc, char **argv)
{
    TTokenArr *tokens = (TTokenArr *)malloc(sizeof(TTokenArr));
    tokens->capacity = argc;
    tokens->tokens = (TToken *)malloc(tokens->capacity * sizeof(TToken));
    tokens->length = 0;

    for (int i = 0; i < argc; i++)
    {
        char *value = argv[i];
        if (is_option(value))
        {
            int length = strlen(value);
            for (int optIndex = 1; optIndex < length; optIndex++)
            {
                char optValue = value[optIndex];
                TToken option = get_option_token(optValue);
                append_token(tokens, option);
            }
        }
        else
        {
            TToken arg = get_arg_token(value);
            append_token(tokens, arg);
        }
    }
    return tokens;
}

static int count_args_after_option(TTokenArr *tokens, int start)
{
    int curIndex = start;
    while (curIndex < tokens->length && tokens->tokens[curIndex].type == argType)
    {
        curIndex++;
    }
    return curIndex - start;
}

static char **read_args_after_option(TTokenArr *tokensArr, int start)
{
    int argsCount = count_args_after_option(tokensArr, start);
    char **dest = (char **)malloc(argsCount * sizeof(char *));

    for (int i = 0; i < argsCount; i++)
    {
        dest[i] = tokensArr->tokens[i + start].argValue;
    }
    return dest;
}

static int is_arg(TTokenArr *arr, int index)
{
    if (index >= arr->length || arr->tokens[index].type != argType)
    {
        return false;
    }
    return true;
}

static EArchivatorMode get_mode_by_option(char optValue)
{
    switch (optValue)
    {
    case 'a':
        return archivateMode;
    case 'l':
        return infoMode;
    case 'e':
        return dearchivateMode;
    case 'c':
        return checkMode;
    default:
        return undefinedMode;
    }
}

static int is_mode_option(char optValue)
{
    EArchivatorMode mode = get_mode_by_option(optValue);
    if (mode == undefinedMode)
        return false;
    return true;
}

void print_settings(TSetupSettings *settings)
{
    printf("%d\n", settings->mode);

    printf("files_count = %d\n", settings->filesCount);
    for (int i = 0; i < settings->filesCount; i++)
    {
        printf("file: %s\n", settings->filesToArchivate[i]);
    }
    printf("\n");

    printf("archive path: %s\n", settings->archivePath != NULL ? settings->archivePath : "");
    printf("dest dir: %s\n", settings->destDir != NULL ? settings->destDir : "");

    printf("!!!\n");
    printf("Is error: %d\n", settings->isError);
    printf("message: %s\n", settings->errorMessage);
    printf("!!!\n");
}
static void check_filled_settings_on_valid(TSetupSettings *settings)
{
    if (settings->mode == null)
    {
        snprintf(settings->errorMessage, ERROR_LENGTH, "No archivator mode");
        settings->isError = 1;
        return;
    }
    if (settings->archivePath == NULL)
    {
        snprintf(settings->errorMessage, ERROR_LENGTH, "No archive path");
        settings->isError = 1;
        return;
    }
    if (settings->mode != dearchivateMode && settings->destDir != NULL)
    {
        snprintf(settings->errorMessage, ERROR_LENGTH, "excess option: -d");
        settings->isError = 1;
        return;
    }
}

TSetupSettings *read_setup_settings(int argc, char **argv)
{
    TSetupSettings *settings = get_settings();
    TTokenArr *tokensArr = read_tokens(argc, argv);

    for (int i = 1; i < tokensArr->length; i++)
    {
        TToken token = tokensArr->tokens[i];
        if (token.type == argType)
        {
            // if (settings->mode == null)
            // {
            //     snprintf(settings->errorMessage, ERROR_LENGTH, "Undefined argument: %s", token.argValue);
            //     settings->isError = true;
            //     goto exit;
            // }

            char *archivePath = token.argValue;
            if (settings->archivePath != NULL)
            {
                snprintf(settings->errorMessage, ERROR_LENGTH, "Redefine archive path: %s", token.argValue);
                settings->isError = true;
                goto exit;
            }
            settings->archivePath = token.argValue;
            continue;
        }

        if (token.optionValue == 'f')
        {
            if (settings->dirToArchivate != NULL)
            {
                snprintf(settings->errorMessage, ERROR_LENGTH, "option -f can't be used with option -r");
                settings->isError = true;
                goto exit;
            }
            int filesCount = count_args_after_option(tokensArr, i + 1);
            if (filesCount == 0)
            {
                snprintf(settings->errorMessage, ERROR_LENGTH, "No files to archivate");
                settings->isError = true;
                goto exit;
            }
            settings->filesCount = filesCount;
            settings->filesToArchivate = read_args_after_option(tokensArr, i + 1);
            i += filesCount;
        }
        else if (token.optionValue == 'r')
        {
            if (!is_arg(tokensArr, i + 1))
            {
                snprintf(settings->errorMessage, ERROR_LENGTH, "No dir path to archivate (-r option)");
                settings->isError = true;
                goto exit;
            }
            settings->dirToArchivate = tokensArr->tokens[i + 1].argValue;
            i++;
        }
        else if (token.optionValue == 'd')
        {
            if (!is_arg(tokensArr, i + 1))
            {
                snprintf(settings->errorMessage, ERROR_LENGTH, "No dest path (-d option)");
                settings->isError = true;
                goto exit;
            }
            settings->destDir = tokensArr->tokens[i + 1].argValue;
            i++;
        }
        else if (token.optionValue == 'i')
        {
            settings->withInfo = true;
        }
        else if (is_mode_option(token.optionValue))
        {
            if (settings->mode != null)
            {
                snprintf(settings->errorMessage, ERROR_LENGTH, "Redefine archivator mode, option: -%c", token.optionValue);
                settings->isError = true;
                goto exit;
            }
            settings->mode = get_mode_by_option(token.optionValue);
        }
        else
        {
            snprintf(settings->errorMessage, ERROR_LENGTH, "Undefined option: -%c", token.optionValue);
            settings->isError = true;
            goto exit;
        }
    }
    check_filled_settings_on_valid(settings);
exit:
    free(tokensArr->tokens);
    free(tokensArr);

    // print_settings(settings);
    return settings;
}