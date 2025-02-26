#include "utils.h"


wchar_t *convert_to_wide(const char16_t *str, size_t cnt){
    wchar_t *result = malloc(sizeof(wchar_t) * (cnt+1));
    for(size_t i=0; i<cnt; ++i) result[i] = (wchar_t) str[i];
    result[cnt] = L'\0';

    return result;
}


size_t utf16_strlen(const char16_t *str){
    size_t length = 0;
    while(str[length]) ++length;

    return length;
}


wchar_t **tokenize(wchar_t *string, wchar_t *delimiter, size_t *number_of_tokens){
    wchar_t *temp = malloc(sizeof(wchar_t) * (wcslen(string)+1));
    wcscpy(temp, string);

    wchar_t *token, *buffer;
    size_t cnt = 0;
    token = wcstok(temp, delimiter, &buffer);
    while(token) token = wcstok(NULL, delimiter, &buffer), cnt++;

    free(temp);

    temp = malloc(sizeof(wchar_t) * (wcslen(string)+1));
    wcscpy(temp, string);

    wchar_t **tokens = malloc(sizeof(wchar_t *) * cnt);
    memset(tokens, 0, sizeof(wchar_t *) * cnt);
    token = wcstok(temp, delimiter, &buffer);
    for(size_t i=0; i<cnt; ++i){
        tokens[i] = malloc(sizeof(wchar_t) * (wcslen(token)+1));
        wcscpy(tokens[i], token);

        token = wcstok(NULL, delimiter, &buffer);
    }

    free(temp);

    *number_of_tokens = cnt;

    return tokens;
}


void solve(wchar_t *path, struct deque *dq){
    size_t path_number_of_tokens = 0;
    wchar_t **path_tokens = tokenize(path, L"/", &path_number_of_tokens);

    for(size_t i=0; i<path_number_of_tokens; ++i){
        wchar_t *token = path_tokens[i];
        char *c_string = make_short_name(token);

        // char *c_string = malloc(sizeof(char) * (wcslen(token)+1));
        // for(size_t j=0; j<wcslen(token); ++j) c_string[j] = token[j];
        // c_string[wcslen(token)] = '\0';

        list_directory(get_back_deque(dq));

        bool flag = false;
        for(size_t j=0; j<entries.count; ++j){
            struct entry *e = (struct entry *) entries.data[j];

            if(!(e->attributes & ATTR_DIRECTORY)) continue;

            if(e->long_name!=NULL && !wcscmp(e->long_name, token)){
                flag = true;
                push_back_deque(dq, e->first_cluster);

                break;
            }
            else if(!strncmp(e->short_name, c_string, 11)){
                flag = true;

                if(!wcscmp(token, L".")){
                }
                else if(!wcscmp(token, L"..")){
                    pop_back_deque(dq);
                }
                else{
                    push_back_deque(dq, e->first_cluster);
                }

                break;
            }
        }

        free(c_string);

        assert(flag);
    }

    for(size_t i=0; i<path_number_of_tokens; ++i) free(path_tokens[i]);
    free(path_tokens);
}