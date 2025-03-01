#include "utils.h"


char **tokenize(const char *string, const char *delimiter, size_t *number_of_tokens){
    char *temp1 = malloc(sizeof(char) * (strlen(string)+1));
    strcpy(temp1, string);

    char *token = strtok(temp1, delimiter);
    size_t cnt = 0;
    while(token != NULL){
        token = strtok(NULL, delimiter), cnt++;
    }

    free(temp1);

    char *temp2 = malloc(sizeof(char) * (strlen(string)+1));
    strcpy(temp2, string);

    char **tokens = malloc(sizeof(char *) * cnt);
    memset(tokens, 0, sizeof(char *) * cnt);
    token = strtok(temp2, delimiter);
    for(size_t i=0; i<cnt; ++i){
        assert(token != NULL);

        tokens[i] = malloc(sizeof(char) * (strlen(token)+1));
        strcpy(tokens[i], token);

        token = strtok(NULL, delimiter);
    }

    free(temp2);

    *number_of_tokens = cnt;

    return tokens;
}


void solve(const char *path, struct deque *dq){
    size_t path_number_of_tokens = 0;
    char **path_tokens = tokenize(path, "/", &path_number_of_tokens);

    for(size_t i=0; i<path_number_of_tokens; ++i){
        char *token = path_tokens[i];
        char *short_name = make_short_name(token);


        bool flag = false;
        struct deque *entries = get_entries(get_back_deque(&dq));
        for(size_t j=0; j<entries->count; ++j){
            struct entry *e = (struct entry *) entries->data[j];

            if(!(e->attributes & ATTR_DIRECTORY)) continue;

            if(e->long_name!=NULL && !strcmp(e->long_name, token)){
                flag = true;
                push_back_deque(&dq, e->first_cluster);

                break;
            }
            else if(!strcmp(e->short_name, short_name)){
            // else if(!strncmp(e->short_name, short_name, 11)){
                flag = true;

                if(!strcmp(token, ".")){
                }
                else if(!strcmp(token, "..")){
                    pop_back_deque(&dq);
                }
                else{
                    push_back_deque(&dq, e->first_cluster);
                }

                break;
            }
        }

        free(short_name);
        delete_entries(entries);

        assert(flag);
    }

    for(size_t i=0; i<path_number_of_tokens; ++i) free(path_tokens[i]);
    free(path_tokens);
}